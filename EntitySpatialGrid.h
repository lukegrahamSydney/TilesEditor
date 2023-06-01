#ifndef ENTITYSPATIALGRIDH
#define ENTITYSPATIALGRIDH

#include <QList>
#include <QSet>
#include <cmath>
#include <stdint.h>

#include "IEntitySpatialMap.h"
#include "ISpatialMapItem.h"


namespace TilesEditor
{
    template <typename  T>
    class EntitySpatialGrid :
        public IEntitySpatialMap<T>
    {
    private: 
        double m_x;
        double m_y;

        int m_width;
        int m_height;

        int m_cellWidth;
        int m_cellHeight;

        int m_hcount;
        int m_vcount;

        QSet<T* >* m_grid;


    public:
        EntitySpatialGrid(double x, double y, int mapWidth, int mapHeight, int cellWidth = 256, int cellHeight = 256)
        {
            m_x = x;
            m_y = y;
            m_hcount = (int)std::ceil((double)mapWidth / cellWidth);
            m_vcount = (int)std::ceil((double)mapHeight / cellHeight);

            m_width = mapWidth;
            m_height = mapHeight;

            m_cellWidth = cellWidth;
            m_cellHeight = cellHeight;

            int count = m_hcount * m_vcount;
            m_grid = new QSet<T*>[count];
        }

        ~EntitySpatialGrid()
        {
            delete[]m_grid;
        }

        int search(const IRectangle& rect, bool accurate, QList<T*>& output, bool (*f)(T*, void* userData), void* userData)
        {
            static uint64_t currentSearchIndex = 0;

            ++currentSearchIndex;

            auto x = rect.getX() - m_x;
            auto y = rect.getY() - m_y;

            int left = (int)std::max(std::floor(x / m_cellWidth), 0.0);
            int top = (int)std::max(std::floor(y / m_cellHeight), 0.0);

            int right = (int)std::min(std::ceil((x + rect.getWidth()) / m_cellWidth), (double)m_hcount);
            int bottom = (int)std::min(std::ceil((y + rect.getHeight()) / m_cellHeight), (double)m_vcount);

            int count = 0;
            for (int y = top; y < bottom; ++y)
            {
                for (int x = left; x < right; ++x)
                {
                    auto& cell = m_grid[y * m_hcount + x];
                    if (accurate)
                    {

                        for (const auto& entity : cell)
                        {
                            if (currentSearchIndex != entity->m_spatialGridSearchIndex && (f == nullptr || f(entity, userData)))
                            {

                                entity->m_spatialGridSearchIndex = currentSearchIndex;
                                if (entity->intersects(rect))
                                {

                                    output.append(entity);
                                    ++count;
                                }

                            }
                            else entity->m_spatialGridSearchIndex = currentSearchIndex;
                        }
                    }
                    else
                    {

                        for (const auto& entity : cell) {
                            if (currentSearchIndex != entity->m_spatialGridSearchIndex)
                            {
                                if (f == nullptr || f(entity, userData))
                                {
                                    entity->m_spatialGridSearchIndex = currentSearchIndex;
                                    output.append(entity);
                                    ++count;
                                }
                            }
                        }

                    }
                }
            }


            return count;
        }


        int search(const IRectangle& rect, bool accurate, QSet<T*>& output, bool (*f)(T*, void* userData), void* userData)
        {
            static uint64_t currentSearchIndex = 0;

            ++currentSearchIndex;

            auto x = rect.getX() - m_x;
            auto y = rect.getY() - m_y;

            int left = (int)std::max(std::floor(x / m_cellWidth), 0.0);
            int top = (int)std::max(std::floor(y / m_cellHeight), 0.0);

            int right = (int)std::min(std::ceil((x + rect.getWidth()) / m_cellWidth), (double)m_hcount);
            int bottom = (int)std::min(std::ceil((y + rect.getHeight()) / m_cellHeight), (double)m_vcount);

            int count = 0;
            for (int y = top; y < bottom; ++y)
            {
                for (int x = left; x < right; ++x)
                {
                    auto& cell = m_grid[y * m_hcount + x];
                    if (accurate)
                    {

                        for (const auto& entity : cell)
                        {
                            if (currentSearchIndex != entity->m_spatialGridSearchIndex && (f == nullptr || f(entity, userData)))
                            {

                                entity->m_spatialGridSearchIndex = currentSearchIndex;
                                if (entity->intersects(rect))
                                {

                                    output.insert(entity);
                                    ++count;
                                }

                            }
                            else entity->m_spatialGridSearchIndex = currentSearchIndex;
                        }
                    }
                    else
                    {

                        for (const auto& entity : cell) {
                            if (currentSearchIndex != entity->m_spatialGridSearchIndex)
                            {
                                if (f == nullptr || f(entity, userData))
                                {
                                    entity->m_spatialGridSearchIndex = currentSearchIndex;
                                    output.insert(entity);
                                    ++count;
                                }
                            }
                        }

                    }
                }
            }


            return count;
        }

        T* searchFirst(const IRectangle& rect, bool accurate, bool (*f)(T*, void* userData), void* userData)
        {
            auto x = rect.getX() - m_x;
            auto y = rect.getY() - m_y;

            int left = (int)std::max(std::floor(x / m_cellWidth), 0.0);
            int top = (int)std::max(std::floor(y / m_cellHeight), 0.0);

            int right = (int)std::min(std::ceil((x + rect.getWidth()) / m_cellWidth), (double)m_hcount);
            int bottom = (int)std::min(std::ceil((y + rect.getHeight()) / m_cellHeight), (double)m_vcount);

            for (int y = top; y < bottom; ++y)
            {
                for (int x = left; x < right; ++x)
                {
                    auto& cell = m_grid[y * m_hcount + x];
                    if (accurate)
                    {

                        for (auto& entity : cell)
                        {

                            if ((f == nullptr || f(entity, userData)) && entity->intersects(rect)) {
                                return entity;
                            }
                        }
                    }
                    else
                    {

                        for (const auto& entity : cell)
                        {
                            if (f == nullptr || f(entity, userData))
                                return entity;
                        }

                    }
                }
            }


            return nullptr;
        }

        T* entityAt(double _x, double _y)
        {
            auto x = _x - m_x;
            auto y = _y - m_y;

            Rectangle rect(x, y, 1, 1);
            int left = (int)std::max(std::floor(x / m_cellWidth), 0.0);
            int top = (int)std::max(std::floor(y / m_cellHeight), 0.0);

            if (left >= 0 && top >= 0 && left < m_hcount && top < m_vcount)
            {
                auto& cell = m_grid[top * m_hcount + left];
                for (auto& entity : cell)
                {
                    if (entity->intersects(rect))
                        return entity;
                }
            }
            return nullptr;
        }

        void add(T* entity)
        {
            if (entity->m_spatialGridAdded) {
                return;
            }

            auto x = entity->getX() - m_x;
            auto y = entity->getY() - m_y;

            entity->m_spatialGridAdded = true;
            entity->m_spacialGridLeft = (int)std::max(std::floor(x / m_cellWidth), 0.0);
            entity->m_spacialGridTop = (int)std::max(std::floor(y / m_cellHeight), 0.0);
            entity->m_spacialGridRight = (int)std::min(std::ceil((x + entity->getWidth()) / m_cellWidth), (double)m_hcount);
            entity->m_spacialGridBottom = (int)std::min(std::ceil((y + entity->getHeight()) / m_cellHeight), (double)m_vcount);


            for (auto y = entity->m_spacialGridTop; y < entity->m_spacialGridBottom; ++y)
            {
                for (auto x = entity->m_spacialGridLeft; x < entity->m_spacialGridRight; ++x)
                {
                    auto& cell = m_grid[y * m_hcount + x];
                    cell.insert(entity);
                }
            }
        }

        void remove(T* entity)
        {
            for (auto y = entity->m_spacialGridTop; y < entity->m_spacialGridBottom; ++y)
            {
                for (auto x = entity->m_spacialGridLeft; x < entity->m_spacialGridRight; ++x)
                {
                    auto& cell = m_grid[y * m_hcount + x];
                    cell.remove(entity);
                }
            }
            entity->m_spacialGridTop = entity->m_spacialGridLeft = entity->m_spacialGridBottom = entity->m_spacialGridRight = 0;
            entity->m_spatialGridAdded = false;
        }

        void updateEntity(T* entity)
        {
            if (!entity->m_spatialGridAdded)
                return;

            auto x = entity->getX() - m_x;
            auto y = entity->getY() - m_y;

            int left = (int)std::max(std::floor(x / m_cellWidth), 0.0);
            int top = (int)std::max(std::floor(y / m_cellHeight), 0.0);
            int right = (int)std::min(std::ceil((x + entity->getWidth()) / m_cellWidth), (double)m_hcount);
            int bottom = (int)std::min(std::ceil((y + entity->getHeight()) / m_cellHeight), (double)m_vcount);

            //x/y/width/height has changed
            if (left != entity->m_spacialGridLeft ||
                top != entity->m_spacialGridTop ||
                right != entity->m_spacialGridRight ||
                bottom != entity->m_spacialGridBottom)
            {
                //Remove then re-add
                remove(entity);
                entity->m_spacialGridLeft = left;
                entity->m_spacialGridTop = top;
                entity->m_spacialGridRight = right;
                entity->m_spacialGridBottom = bottom;
                entity->m_spatialGridAdded = true;

                for (auto y = entity->m_spacialGridTop; y < entity->m_spacialGridBottom; ++y)
                {
                    for (auto x = entity->m_spacialGridLeft; x < entity->m_spacialGridRight; ++x)
                    {
                        auto& cell = m_grid[y * m_hcount + x];
                        cell.insert(entity);
                    }
                }
            }
        }
    };
}
#endif
