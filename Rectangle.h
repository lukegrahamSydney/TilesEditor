#ifndef RECTANGLEH
#define RECTANGLEH

#include <algorithm>

namespace TilesEditor
{
	class IRectangle
	{
	public:
		virtual double getX() const = 0;
		virtual double getY() const = 0;
		virtual int getWidth() const = 0;
		virtual int getHeight() const = 0;

		virtual void reset(double x, double y, int width, int height) {}

		double getRight() const {
			return getX() + getWidth();
		}

		double getBottom() const {
			return getY() + getHeight();
		}

		double getCenterX() const {
			return getX() + getWidth() / 2.0;
		}

		double getCenterY() const {
			return getY() + getHeight() / 2.0;
		}

		bool intersects(const IRectangle& other) const {
			return other.getX() + other.getWidth() > this->getX() && other.getY() + other.getHeight() > this->getY() &&
				other.getX() < this->getX() + this->getWidth() && other.getY() < this->getY() + this->getHeight();
		}

		bool inside(const IRectangle& other) const {
			return other.getX() >= this->getX() && other.getY() >= this->getY() &&
				other.getRight() <= this->getRight() &&
				other.getBottom() <= this->getBottom();
		}

		bool getOverlap(const IRectangle& other, IRectangle& out) const
		{
			auto x = std::max(this->getX(), other.getX());
			auto y = std::max(this->getY(), other.getY());

			out.reset(x,
				y,
				std::min(this->getRight(), other.getRight()) - x,
				std::min(this->getBottom(), other.getBottom()) - y);


			return true;
		}

		inline bool operator==(const IRectangle& other) {
			return (getX() == other.getX() && getY() == other.getY() && getWidth() == other.getWidth() && getHeight() == other.getHeight());
		}
	};

	class Rectangle : public IRectangle
	{
	protected:
		double	m_x,
			m_y;

		int		m_width,
			m_height;

	public:
		Rectangle() :
			m_x(0.0), m_y(0.0), m_width(0), m_height(0) {}
		Rectangle(double x, double y, int width, int height) :
			m_x(x), m_y(y), m_width(width), m_height(height) {}
		Rectangle(const IRectangle& other) :
			m_x(other.getX()), m_y(other.getY()), m_width(other.getWidth()), m_height(other.getHeight()) {}

		double getX() const {
			return m_x;
		}

		double getY() const {
			return m_y;
		}

		int getWidth() const {
			return m_width;
		}

		int getHeight() const {
			return m_height;
		}

		void setX(double val) {
			m_x = val;
		}

		void setY(double val) {
			m_y = val;
		}

		void setWidth(int val) {
			m_width = val;
		}

		void setHeight(int val) {
			m_height = val;
		}

		void reset(double x, double y, int width, int height) override {
			m_x = x;
			m_y = y;
			m_width = width;
			m_height = height;
		}
	};
};
#endif
