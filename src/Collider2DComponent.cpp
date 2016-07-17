// Onut includes
#include <onut/Collider2DComponent.h>
#include <onut/Entity.h>
#include <onut/SceneManager.h>
#include <onut/Timing.h>

// Third parties
#include <Box2D/Box2D.h>

namespace onut
{
    Collider2DComponent::~Collider2DComponent()
    {
        destroyBody();
    }

    const Vector2& Collider2DComponent::getSize() const
    {
        return m_size;
    }

    void Collider2DComponent::setSize(const Vector2& size)
    {
        if (m_size == size) return;
        m_size = size;
        if (m_pBody)
        {
            destroyBody();
            createBody();
        }
    }

    bool Collider2DComponent::getTrigger() const
    {
        return m_trigger;
    }

    void Collider2DComponent::setTrigger(bool trigger)
    {
        if (m_trigger == trigger) return;
        m_trigger = trigger;
        if (m_pBody)
        {
            destroyBody();
            createBody();
        }
    }

    float Collider2DComponent::getPhysicScale() const
    {
        return m_physicScale;
    }

    void Collider2DComponent::setPhysicScale(float scale)
    {
        m_physicScale = scale;
    }

    const Vector2& Collider2DComponent::getVelocity() const
    {
        return m_velocity;
    }

    void Collider2DComponent::setVelocity(const Vector2& velocity)
    {
        m_velocity = velocity;
    }       
    
    void Collider2DComponent::createBody()
    {
        auto& pEntity = getEntity();
        auto& pSceneManager = pEntity->getSceneManager();
        auto pPhysic = pSceneManager->getPhysic2DWorld();
        Vector2 pos = pEntity->getWorldTransform().Translation();

        b2BodyDef bodyDef;
        bodyDef.type = pEntity->isStatic() ? b2_staticBody : (m_trigger ? b2_kinematicBody : b2_dynamicBody);
        bodyDef.position.Set(pos.x / m_physicScale, pos.y / m_physicScale);

        m_pBody = pPhysic->CreateBody(&bodyDef);
        m_pBody->SetUserData(this);
        m_pBody->SetFixedRotation(true);

        b2PolygonShape box;
        box.SetAsBox(m_size.x / 2.f / m_physicScale, m_size.y / 2.f / m_physicScale);

        if (pEntity->isStatic())
        {
            m_pBody->CreateFixture(&box, 0.0f);
        }
        else if (m_trigger)
        {
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &box;
            fixtureDef.isSensor = true;
            m_pBody->CreateFixture(&fixtureDef);
        }
        else
        {
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &box;
            fixtureDef.friction = 0.0f;
            fixtureDef.density = 1.0f;
            m_pBody->CreateFixture(&fixtureDef);
        }
    }

    void Collider2DComponent::destroyBody()
    {
        if (m_pBody)
        {
            auto& pEntity = getEntity();
            if (pEntity)
            {
                auto& pSceneManager = pEntity->getSceneManager();
                if (pSceneManager)
                {
                    auto pPhysic = pSceneManager->getPhysic2DWorld();
                    if (pPhysic)
                    {
                        pPhysic->DestroyBody(m_pBody);
                    }
                }
            }
        }
    }

    void Collider2DComponent::onCreate()
    {
        createBody();
    }

    void Collider2DComponent::onUpdate()
    {
        if (m_pBody)
        {
            if (m_trigger)
            {
                Vector2 position = getEntity()->getLocalTransform().Translation();
                m_pBody->SetTransform(b2Vec2(position.x / m_physicScale, position.y / m_physicScale), 0.0f);
            }
            else
            {
                auto b2Position = m_pBody->GetPosition();
                Vector2 currentPosition(b2Position.x * m_physicScale, b2Position.y * m_physicScale);
                getEntity()->setLocalTransform(Matrix::CreateTranslation(currentPosition));
                Vector2 vel = m_velocity / m_physicScale;
                m_pBody->SetLinearVelocity(b2Vec2(vel.x, vel.y));
            }
        }
    }

    void Collider2DComponent::teleport(const Vector2& position)
    {
        if (m_pBody)
        {
            b2Vec2 b2Pos(position.x / m_physicScale, position.y / m_physicScale);
            getEntity()->setLocalTransform(Matrix::CreateTranslation(position));
            m_pBody->SetTransform(b2Pos, 0.0f);
        }
    }
};
