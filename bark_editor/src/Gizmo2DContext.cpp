#include <onut/ActionManager.h>
#include <onut/PrimitiveBatch.h>
#include <onut/SpriteBatch.h>
#include "Gizmo2DContext.h"
#include "Entity.h"
#include "GUIContext.h"
#include "Project.h"
#include "globals.h"

void Gizmo2DContext::reset()
{
    inv_scale = (1.0f / scale) * DOTTED_SCALE;

    multi_select_mins.x = std::numeric_limits<float>::max();
    multi_select_mins.y = std::numeric_limits<float>::max();
    multi_select_maxs.x = std::numeric_limits<float>::min();
    multi_select_maxs.y = std::numeric_limits<float>::min();
}

void Gizmo2DContext::drawDottedRect(const Matrix& transform, const Vector2& size, const Vector2& origin)
{
    auto invOrigin = Vector2(1.f - origin.x, 1.f - origin.y);

    Vector2 tl(-size.x * origin.x, -size.y * origin.y);
    Vector2 bl(-size.x * origin.x, size.y * invOrigin.y);
    Vector2 br(size.x * invOrigin.x, size.y * invOrigin.y);
    Vector2 tr(size.x * invOrigin.x, -size.y * origin.y);

    single_select_rect.x = tl.x;
    single_select_rect.y = tl.y;
    single_select_rect.z = br.x - tl.x;
    single_select_rect.w = br.y - tl.y;

    single_select_corners[0] = Vector2::Transform(tl, transform);
    single_select_corners[1] = Vector2::Transform(bl, transform);
    single_select_corners[2] = Vector2::Transform(br, transform);
    single_select_corners[3] = Vector2::Transform(tr, transform);

    for (int i = 0; i < 4; ++i)
    {
        const auto& p1 = single_select_corners[i];
        const auto& p2 = single_select_corners[(i + 1) % 4];

        pb->draw(p1, Color::White, p1 * inv_scale);
        pb->draw(p2, Color::White, p2 * inv_scale);

        multi_select_mins.x = std::min(p1.x, multi_select_mins.x);
        multi_select_mins.y = std::min(p1.y, multi_select_mins.y);
        multi_select_maxs.x = std::max(p1.x, multi_select_maxs.x);
        multi_select_maxs.y = std::max(p1.y, multi_select_maxs.y);
    }

    single_select_transform = transform;
}

void Gizmo2DContext::drawOrigin(const Matrix& transform)
{
    auto pos3 = transform.Translation();
    auto pos2 = Vector2(pos3.x, pos3.y);

    auto offset = ORIGIN_SIZE * scale;
    auto neg_offset = (ORIGIN_SIZE + 1.0f) * scale;

    pb->draw(Vector2(pos2.x, pos2.y - neg_offset));
    pb->draw(Vector2(pos2.x, pos2.y + offset));
    pb->draw(Vector2(pos2.x - neg_offset, pos2.y));
    pb->draw(Vector2(pos2.x + offset, pos2.y));
}

static eUICursorType getSizeCursorForDir(Vector2 dir)
{
    dir.Normalize();

    static const float epsilon = 0.92387953251128675612818318939679f; // 22.5 deg

    // Starting with north direction
    if (dir.y < -epsilon) return eUICursorType::SizeNS;
    if (dir.y > epsilon) return eUICursorType::SizeNS;
    if (dir.x < -epsilon) return eUICursorType::SizeEW;
    if (dir.x > epsilon) return eUICursorType::SizeEW;
    if (dir.x > 0.0f && dir.y < 0.0f) return eUICursorType::SizeNESW;
    if (dir.x > 0.0f && dir.y > 0.0f) return eUICursorType::SizeNWSE;
    if (dir.x < 0.0f && dir.y > 0.0f) return eUICursorType::SizeNESW;
    return eUICursorType::SizeNWSE;
}

static eUICursorType getRotCursorForDir(Vector2 dir)
{
    dir.Normalize();

    static const float epsilon = 0.92387953251128675612818318939679f; // 22.5 def

    // Starting with north direction
    if (dir.y < -epsilon) return eUICursorType::RotN;
    if (dir.y > epsilon) return eUICursorType::RotS;
    if (dir.x < -epsilon) return eUICursorType::RotW;
    if (dir.x > epsilon) return eUICursorType::RotE;
    if (dir.x > 0.0f && dir.y < 0.0f) return eUICursorType::RotNE;
    if (dir.x > 0.0f && dir.y > 0.0f) return eUICursorType::RotSE;
    if (dir.x < 0.0f && dir.y > 0.0f) return eUICursorType::RotSW;
    return eUICursorType::RotNW;
}

bool Gizmo2DContext::drawHandle(const Vector2& pos, const Vector2& dir, eUICursorType* cursor)
{
    auto handle_dist = 8.0f * scale;
    auto adj_pos = pos + dir * handle_dist;

    oSpriteBatch->drawSprite(nullptr, adj_pos, Color::Black, 0.0f, 8.0f * scale);
    oSpriteBatch->drawSprite(nullptr, adj_pos, Color::White, 0.0f, 6.0f * scale);

    auto hover_eps = 12.0f * scale;
    if (Rect(adj_pos.x - hover_eps, adj_pos.y - hover_eps, hover_eps * 2.0f, hover_eps * 2.0f).Contains(mouse_pos))
    {
        *cursor = getSizeCursorForDir(dir);
    }

    return false;
}

void Gizmo2DContext::doMoveLogic(const std::vector<EntityRef>& entities, GUIContext* ctx)
{
    ctx->cursor_type = eUICursorType::SizeAll;

    auto transforms_after = move_ctx.entity_transforms_on_down;
    auto diff             = mouse_pos - move_ctx.mouse_pos_on_down;
    for (int i = 0, len = (int)entities.size(); i < len; ++i)
    {
        const auto& entity      = entities[i];
        auto&       transform   = transforms_after[i];

        transform.Translation(transform.Translation() + Vector3(diff));
        entity->setLocalTransform(transform);
    }

    if (ctx->left_button.clicked)
    {
        state               = eGizmo2DState::None;
        ctx->cursor_type    = eUICursorType::Arrow;

        // Add undo action
        auto entities_copy      = entities;
        auto transforms_before  = move_ctx.entity_transforms_on_down;
        auto scene              = g_project->getSceneViewFilename();

        oActionManager->addAction("Move", [=]()
        {
            if (!g_project->openScene(scene)) return; // This shouldn't happen
            for (int i = 0, len = (int)entities_copy.size(); i < len; ++i)
            {
                const auto& entity      = entities[i];
                auto&       transform   = transforms_after[i];
                entity->setLocalTransform(transform);
            }
        }, [=]()
        {
            if (!g_project->openScene(scene)) return; // This shouldn't happen
            for (int i = 0, len = (int)entities_copy.size(); i < len; ++i)
            {
                const auto& entity      = entities[i];
                auto&       transform   = transforms_before[i];
                entity->setLocalTransform(transform);
            }
        });

        return;
    }
}

void Gizmo2DContext::drawSelected(const std::vector<EntityRef>& entities, GUIContext* ctx)
{
    if (entities.empty()) return;

    if (state == eGizmo2DState::Move)
    {
        doMoveLogic(entities, ctx);
    }

    // We handle things differently if there is only 1 entity selected vs many
    if (entities.size() == 1)
    {
        Vector2 right = single_select_transform.Right();
        Vector2 down  = single_select_transform.Forward(); // This is +Y, so our down into 2D screen

        right.Normalize();
        down.Normalize();

        // Draw grab handles
        eUICursorType desired_cursor = eUICursorType::Arrow;
        if (state != eGizmo2DState::Move && state != eGizmo2DState::Rotate)
        {
            oSpriteBatch->begin(transform);
            drawHandle(single_select_corners[0], -right - down, &desired_cursor);
            drawHandle((single_select_corners[0] + single_select_corners[1]) * 0.5f, -right, &desired_cursor);
            drawHandle(single_select_corners[1], -right + down, &desired_cursor);
            drawHandle((single_select_corners[1] + single_select_corners[2]) * 0.5f, down, &desired_cursor);
            drawHandle(single_select_corners[2], right + down, &desired_cursor);
            drawHandle((single_select_corners[2] + single_select_corners[3]) * 0.5f, right, &desired_cursor);
            drawHandle(single_select_corners[3], right - down, &desired_cursor);
            drawHandle((single_select_corners[3] + single_select_corners[0]) * 0.5f, -down, &desired_cursor);
            oSpriteBatch->end();

            if (ctx->rect.Contains(ctx->mouse))
            {
                auto inv_select_transform   = single_select_transform.Invert();
                auto local_mouse_pos        = Vector2::Transform(mouse_pos, inv_select_transform);

                ctx->cursor_type = desired_cursor;
                if (single_select_rect.Contains(local_mouse_pos))
                {
                    ctx->cursor_type = eUICursorType::SizeAll;

                    if (ctx->left_button.just_down)
                    {
                        state = eGizmo2DState::Move;
                        move_ctx.mouse_pos_on_down = mouse_pos;
                        move_ctx.entity_transforms_on_down.clear();
                        for (const auto& entity : entities)
                        {
                            move_ctx.entity_transforms_on_down.push_back(entity->getWorldTransform());
                        }
                    }
                }
                else if (desired_cursor == eUICursorType::Arrow)
                {
                    if (single_select_rect.Grow(100.0f * scale).Contains(local_mouse_pos))
                    {
                        auto dir = local_mouse_pos - single_select_rect.Center();
                        ctx->cursor_type = getRotCursorForDir(dir);
                    }
                }
            }
        }
    }
    else
    {
        Vector2 corners[4] = {
            multi_select_mins,
            Vector2(multi_select_mins.x, multi_select_maxs.y),
            multi_select_maxs,
            Vector2(multi_select_maxs.x, multi_select_mins.y)
        };

        for (int i = 0; i < 4; ++i)
        {
            const auto& p1 = corners[i];
            const auto& p2 = corners[(i + 1) % 4];

            pb->draw(p1, Color(1, 1, 0, 1), p1 * inv_scale);
            pb->draw(p2, Color(1, 1, 0, 1), p2 * inv_scale);
        }

        if (ctx->rect.Contains(ctx->mouse))
        {
            ctx->cursor_type = eUICursorType::Arrow;
            if (Rect(multi_select_mins, multi_select_maxs - multi_select_mins).Contains(mouse_pos))
            {
                ctx->cursor_type = eUICursorType::SizeAll;
            }
        }
    }
}
