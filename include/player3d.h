#ifndef __PLAYER3D_H__
#define __PLAYER3D_H__

#include "entity3d.h"
#include "simple_json.h"

/**
 * @brief spawn a new player entity
 * @param position place the player here
 * @return a pointer to the player entity
 */
Entity *player_new(Vector3D position);

/**
 * @brief get a pointer to the player entity
 * @return a pointer to the player entity
 */
Entity *player_get();

/**
 * @brief sets the player position to the one specified
 * @note risk of solid collision
 * @param position the new position for the player
 */
void player_set_position(Vector3D position);

Entity *player_spawn(Vector3D position);

//SJson *args

#endif
