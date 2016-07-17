#include "Door.h"
#include "DoorTraverser.h"

#include <onut/Collider2DComponent.h>
#include <onut/Entity.h>
#include <onut/SceneManager.h>
#include <onut/Sound.h>
#include <onut/TiledMapComponent.h>

extern OAnimFloat m_fadeAnim;

void DoorTraverser::onCreate()
{
    m_pTiledMapComponent = getParentComponent<OTiledMapComponent>();
}

void DoorTraverser::onTriggerEnter(const OCollider2DComponentRef& pCollider)
{
    auto pDoor = pCollider->getComponent<Door>();
    if (pDoor)
    {
        if (pDoor->getOpen())
        {
            // Traverse
            auto pTarget = pDoor->getTarget();
            if (pTarget)
            {
                auto pTargetDoor = pTarget->getComponent<Door>();
                auto pCollider = getComponent<OCollider2DComponent>();
                if (pTargetDoor)
                {
                    auto targetMapPos = pTargetDoor->getMapPos();
                    Vector2 targetPos = pTargetDoor->getLocalTransform().Translation();
                    if (m_pTiledMapComponent->getPassable(Point(targetMapPos.x - 1, targetMapPos.y)))
                    {
                        targetPos.x -= 12.0f;
                    }
                    else if (m_pTiledMapComponent->getPassable(Point(targetMapPos.x + 1, targetMapPos.y)))
                    {
                        targetPos.x += 12.0f;
                    }
                    else if (m_pTiledMapComponent->getPassable(Point(targetMapPos.x, targetMapPos.y - 1)))
                    {
                        targetPos.y -= 12.0f;
                    }
                    else if (m_pTiledMapComponent->getPassable(Point(targetMapPos.x, targetMapPos.y + 1)))
                    {
                        targetPos.y += 12.0f;
                    }

                    auto pEntity = getEntity();
                    auto pSceneManager = pEntity->getSceneManager();

                    // Pause, fade out then teleport
                    pSceneManager->setPause(true);
                    m_fadeAnim.queue(0.0f, .5f, OTweenLinear, 
                                     [pEntity, pSceneManager, pCollider, targetPos]
                    {
                        if (pCollider)
                        {
                            pCollider->teleport(targetPos);
                        }
                        else
                        {
                            pEntity->setLocalTransform(Matrix::CreateTranslation(targetPos));
                        }
                        pSceneManager->setPause(false);
                        pSceneManager->update();
                        pSceneManager->setPause(true);
                    });

                    // Fade in then unpause
                    m_fadeAnim.queue(1.0f, .5f, OTweenLinear, 
                                     [pSceneManager]
                    {
                        pSceneManager->setPause(false);
                    });

                    m_fadeAnim.play();
                }
            }
        }
        else
        {
            // Open the door, and it's target
            if (!pDoor->getOpen())
            {
                pDoor->setOpen(true);

                auto pSound = pDoor->getOpenSound();
                if (pSound) pSound->play();

                auto pTarget = pDoor->getTarget();
                if (pTarget)
                {
                    auto pTargetDoor = pTarget->getComponent<Door>();
                    if (pTargetDoor)
                    {
                        pTargetDoor->setOpen(true);
                    }
                }
            }
        }
    }
}
