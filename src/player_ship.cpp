#include "player_ship.h"
#include <bx/math.h>


void PlayerShip::computeModelMatrix()
{
    mat4 mtxTrans, mtxRot, mtxScale;
    bx::mtxTranslate(mtxTrans, pos.x, pos.y, pos.z);
    bx::mtxQuat(mtxRot, rotation);
    bx::mtxScale(mtxScale, scale.x, scale.y, scale.z);

    bx::mtxMul(mtxModel, mtxTrans, mtxRot);
    bx::mtxMul(mtxModel, mtxModel, mtxScale);
}
