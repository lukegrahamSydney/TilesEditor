#include <QTextStream>
#include <QBuffer>
#include <QThread>
#include "ObjectClass.h"
#include "Image.h"
#include "FileDataLoader.h"


namespace TilesEditor
{
	static QString rstrip(const QString& str) {
		int n = str.size() - 1;
		for (; n >= 0; --n) {
			if (!str.at(n).isSpace()) {
				return str.left(n + 1);
			}
		}
		return "";
	}

	ObjectClass::ObjectClass(const QString& className, const QString& fullPath):
		m_className(className), m_fullPath(fullPath), QTreeWidgetItem(1)
	{
		this->setText(0, className);
	}

	ObjectClass::~ObjectClass()
	{
		for (auto param : m_params)
			delete param;
		m_params.clear();

		for (auto instance : m_instances)
			instance->unsetObjectClass();
		m_instances.clear();

	}

	void ObjectClass::markInstancesModified()
	{
		for (auto instance : m_instances)
			instance->markObjectChanged();
	}
	 
	bool ObjectClass::load(AbstractResourceManager* resourceManager, bool threaded)
	{
		setLoadState(LoadState::STATE_LOADING);

		if (threaded)
		{
			QThread* thread = new QThread();
			auto fileLoader = new FileDataLoader(resourceManager, getFullPath());
			fileLoader->moveToThread(thread);

			connect(thread, &QThread::started, fileLoader, &FileDataLoader::start);
			connect(fileLoader, &FileDataLoader::finished, this, &ObjectClass::loadFileData);
			connect(fileLoader, &FileDataLoader::failed, this, &ObjectClass::loadFileDataFail);
			connect(thread, &QThread::finished, thread, &QThread::deleteLater);
			connect(thread, &QThread::finished, fileLoader, &FileDataLoader::deleteLater);
			thread->start();
		}
		else {
			auto stream = resourceManager->openStreamFullPath(getFullPath(), QIODeviceBase::ReadOnly);
			if (stream)
			{
				load(resourceManager, stream);
				delete stream;
			}
			else setLoadState(LoadState::STATE_FAILED);
			return m_loadState == LoadState::STATE_LOADED;
		}
		return true;

	}

	void ObjectClass::loadFileData(QByteArray fileData)
	{
		auto fileLoader = static_cast<FileDataLoader*>(this->sender());
		QBuffer dataStream(&fileData);
		dataStream.open(QIODeviceBase::ReadOnly);
		load(fileLoader->getResourceManager(),  &dataStream);
	}

	void ObjectClass::loadFileDataFail()
	{
		setLoadState(LoadState::STATE_FAILED);
	}

	bool ObjectClass::load(AbstractResourceManager* resourceManager, QIODevice* stream)
	{ 
		static QRegularExpression paramExpr("PARAM (\\w+) (.+)");
		m_imageLoaded = false;
		setLoadState(LoadState::STATE_LOADING);
		m_imageName = "";
		m_useImageShape = false;
		m_params.clear();
		m_renderMode = RenderMode::Centered;
		m_image = QPixmap();
		m_embedCode = true;

		QTextStream textStream(stream);

		QString line = textStream.readLine();

		if (!line.isNull() && line == "NPC001")
		{
			for (line = textStream.readLine(); !line.isNull(); line = textStream.readLine())
			{
				QStringList words = line.split(' ');

				if (words.size() > 0)
				{
					if (words[0] == "IMAGE" && words.size() >= 2)
					{
						m_imageName = words[1];


					}
					else if (words[0] == "DONTEMBEDCODE")
					{
						m_embedCode = false;
					}
					else if (words[0] == "SCRIPTINGLANGUAGE" && words.size() >= 2)
					{
						auto languageName = words[1].trimmed().toLower();

						if (languageName == "gs1script")
							m_scriptingLanguage = ScriptingLanguage::SCRIPT_GS1;
						else if (languageName == "sgscript")
							m_scriptingLanguage = ScriptingLanguage::SCRIPT_SGSCRIPT;
					}

					else if (words[0] == "IMAGESHAPE" && words.size() >= 5)
					{
						m_useImageShape = true;
						m_imageShape[0] = words[1].toInt();
						m_imageShape[1] = words[2].toInt();
						m_imageShape[2] = words[3].toInt();
						m_imageShape[3] = words[4].toInt();
					} else if (words[0] == "CLIENTCODE")
					{
						if (!textStream.atEnd())
						{
							QString code = "";
							for (auto codeLine = rstrip(textStream.readLine()); !textStream.atEnd(); codeLine = rstrip(textStream.readLine()))
							{
								if (codeLine == "CLIENTCODEEND")
								{
									break;
								}
								else code += codeLine.replace('\t', "    ") + "\n";
							}

							m_clientCode = code;
						}
					}
					else if (words[0] == "SERVERCODE")
					{
						if (!textStream.atEnd())
						{
							QString code = "";
							for (auto codeLine = rstrip(textStream.readLine()); !textStream.atEnd(); codeLine = rstrip(textStream.readLine()))
							{
								if (codeLine == "SERVERCODEEND")
								{
									break;
								}
								else code += codeLine.replace('\t', "    ") + "\n";
							}

							m_serverCode = code;
						}
					}
					else if (words[0] == "PARAM")
					{

						auto match = paramExpr.match(line);
						if (match.hasMatch())
						{
							auto type = match.captured(1);
							auto label = match.captured(2);

							auto paramObject = ObjectClassParamFactory::createParam(type, label);

							if (paramObject != nullptr)
							{
								for (line = textStream.readLine(); !line.isNull(); line = textStream.readLine())
								{
									if (line == "PARAMEND")
										break;

									if (line.startsWith("DEFAULT "))
										paramObject->setDefaultValue(line.mid(strlen("DEFAULT ")));

									else if (line == "NOCOPY")
										paramObject->setCanCopy(false);

									else if (line == "ITEMS" && type == "COMBO")
									{
										auto paramObjectCombo = static_cast<ObjectClassParamList*>(paramObject);

										for (line = textStream.readLine(); !line.isNull(); line = textStream.readLine())
										{
											if (line == "ITEMSEND")
												break;

											auto pos = line.indexOf(';');
											if (pos != -1)
											{
												auto itemName = line.left(pos);
												auto itemValue = line.mid(pos + 1);

												paramObjectCombo->addItem(itemName, itemValue);
											}
										}
									}
									
								}
								m_params.push_back(paramObject);
							}
						}

					}
					else if (words[0] == "EDITORRENDERMODE" && words.size() >= 2)
					{
						auto modeName = words[1].trimmed().toLower();
					
						if (modeName == "centered")
							m_renderMode = RenderMode::Centered;
						else if (modeName == "stretched")
							m_renderMode = RenderMode::Stretched;
						else if(modeName == "tiled")
							m_renderMode = RenderMode::Tiled;
						else if (modeName == "rect") {
							m_renderMode = RenderMode::Rect;
							if (words.size() >= 3)
							{

							}
						}
					}
				}
			}
			setLoadState(LoadState::STATE_LOADED);
			loadImage(resourceManager);
			
			return true;
		}
		
		setLoadState(LoadState::STATE_FAILED);
		return false;
	}

	bool ObjectClass::loadImage(AbstractResourceManager* resourceManager)
	{
		if (getLoadState() == LoadState::STATE_LOADED)
		{
			if (!m_imageLoaded)
			{
				if (!m_imageName.isEmpty())
				{
					m_image = resourceManager->loadPixmap(m_imageName);

					if (!m_image.isNull())
					{
						if (m_useImageShape)
						{
							m_image = m_image.copy(m_imageShape[0], m_imageShape[1], m_imageShape[2], m_imageShape[3]);
						}

						m_imageLoaded = true;
						this->setIcon(0, QIcon(m_image));

						return true;
					}
				}
			}
		}
		return false;
	}

	bool ObjectClass::getImageShape(int* left, int* top, int* width, int* height) const
	{
		if (m_useImageShape)
		{
			*left = m_imageShape[0];
			*top = m_imageShape[1];
			*width = m_imageShape[2];
			*height = m_imageShape[3];
			return true;
		}
		return false;
	}

};