/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include "Video/Screen.h"
#include "gfx_structures.h"
#include <vector>
#include <forward_list>
#include <memory>

// TODO: Create a struct type for portable colors and convert to backend format in each backend's Graphic subclass.
#define R(C) (((C) & 0xFF000000) >> 24)
#define G(C) (((C) & 0x00FF0000) >> 16)
#define B(C) (((C) & 0x0000FF00) >>  8)
#define A(C) (((C) & 0x000000FF) >>  0)

/**
 * When put into an Entity subclass, this allows creating entities via
 * EntityType::push(gfx, entityTypeConstructorArgs...).
 *
 * This can't be put into the Gfx class, because then an attempt to instantiate
 * multiple push() functions with the same signature for different entity types
 * might occur.
 */
#define DEFINE_ENTITY_PUSH(EntityType) \
template<typename... Args> \
static inline void push(Shiro::Gfx& gfx, Args&&... args) { \
    gfx.push(std::make_unique<Shiro::EntityType>(args...)); \
}

namespace Shiro {
    // TODO: Consider moving this somewhere when there's more than just one basic screen type, where each screen type needs its own layout of layers.
    /**
     * This allows scoped enum constants just like `enum class GfxLayer`, but
     * also allows the constants to be implicitly cast to integral values. For
     * this application, it's acceptable to not use the type safety guarantee of
     * `enum class`, because these really can be interpreted as ordered numeric
     * indices, since the Layers class expects layer numbers. This also allows
     * defining a base layer number and offsets from that layer, with those
     * offsets being added to the base layer's constant.
     * -Brandon McGriff
     */
    namespace GfxLayer {
        enum type {
            base, // For the game or menu.
            buttons,
            messages,
            animations,
            emergencyBgDarken,
            emergencyButtons,
            emergencyMessages,
            emergencyAnimations
        };
    }

    struct Graphic {
        virtual ~Graphic();

        virtual void draw(const Screen& screen) const = 0;
    };

    class Gfx;
    class Layers {
        friend Gfx;

    public:
        Layers() = delete;

        Layers(const Screen& screen);

        /**
         * Pushes a new graphic onto the end of a layer's graphic list.
         * You can hold a shared_ptr to the graphic in an entity if you want to reuse it over multiple updates.
         * If you don't hold the pointer, then the graphic will be freed after a draw() or clear() call.
         */
        void push(const size_t layerNum, std::shared_ptr<Graphic> graphic);

    private:
        /**
         * Draw all graphics' layers in increasing order, with layer 0 being the bottom-most.
         * Graphics within a layer are drawn in submission order, back to front.
         */
        void draw();

        /**
         * Clears all layers.
         */
        void clear();

        const Screen& screen;
        std::vector<std::vector<std::shared_ptr<Graphic>>> graphics;
    };

    class Entity {
    public:
        virtual ~Entity();

        /**
         * Returns true if the entity should remain allocated for another update.
         * Returns false if the entity is finished updating and is ready to be freed.
         */
        virtual bool update(Layers& layers) = 0;
    };

    class Gfx {
    public:
        Gfx() = delete;

        Gfx(const Screen& screen);

        /**
         * Pushes a new entity.
         */
        void push(std::unique_ptr<Entity> entity);

        /**
         * Updates all entities.
         * All entities that did not indicate they're ready to be freed will be updated on the next call of update().
         * Don't depend on entities updating in a specific order.
         */
        void update();

        /**
         * Draws all graphics of all layers. Call after an update() call.
         */
        void draw();

        /**
         * Clears all entities and layers.
         */
        void clear();

        /**
         * Clears all layers.
         */
        void clearLayers();

    private:
        const Screen& screen;
        Layers layers;
        std::forward_list<std::unique_ptr<Entity>> entities;
    };
}