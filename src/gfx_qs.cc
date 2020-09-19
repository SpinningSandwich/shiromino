#include "CoreState.h"
#include "DisplayMode.h"
#include "game_qs.h"
#include "GameType.h"
#include "Gfx/Animation.h"
#include "gfx_old.h"
#include "gfx_qs.h"
#include "QRS0.h"
#include "stringtools.h"
#include "random.h"
#include "Timer.h"
#include "SDL.h"
#include <stdlib.h>
#include <string>
#include <vector>
// clang-format off
int piece_colors[26] =
{
    0x00C48B, // I (bright sea green)
    0xE96FDC, // J (bright pink)
    0x007400, // L (dark green)
    0x101010, // X (ash grey)
    0xD18B2F, // S (coral)
    0x0086CE, // Z (ocean foam)
    0x70ECEE, // N (ice)
    0x0BDA51, // G (malachite green)
    0x87C69C, // U (pale emerald)
    0x1200BF, // T (dark blue)
    0x87FF89, // Fa (pale screamin' green)
    0x7F0000, // Fb (dark red)
    0xD3BE00, // P (goldenrod)
    0x8F0A7A, // Q (deep fuchsia)
    0xD22D04, // W (red-orange)
    0xFFB6FF, // Ya (platinum)
    0xDE3163, // Yb (cerise red)
    0x007FFF, // V (azure blue)
    0xD00000,
    0x00A3A3,
    0x0000FF,
    0xFF6000,
    0xEFEF00,
    0xBB3CBB,
    0x00CD00,
    0x808080 // grey
};
// clang-format on

int gfx_drawqs(game_t *g)
{
    if(!g)
        return -1;
    if(!g->origin)
        return -1;

    CoreState *cs = g->origin;
    qrsdata *q = (qrsdata *)(g->data);

    Shiro::PieceDefinition* pd_current = q->p1->def;

    unsigned int drawpiece_next1_flags = DRAWPIECE_PREVIEW;
    if(q->previews.size() > 0)
    {
        if(q->previews[0].qrsID % 18 == 0)
            drawpiece_next1_flags |= DRAWPIECE_IPREVIEW;
    }

    unsigned int drawqrsfield_flags = q->state_flags & GAMESTATE_BIGMODE ? DRAWFIELD_BIG : 0;
    unsigned int drawpiece_flags = q->state_flags & GAMESTATE_BIGMODE ? DRAWPIECE_BIG : 0;

    /*if(q->game_type != SIMULATE_QRS)
    {
        drawpiece_flags |= DRAWPIECE_JEWELED;
    }*/

    SDL_Texture *font = cs->assets->font.tex;
    SDL_Texture *tets_dark_qs = cs->assets->tets_dark_qs.tex;
    //SDL_Texture *tets_jeweled = cs->assets->tets_jeweled.tex;

    SDL_Rect palettesrc = { 0, 0, 16, 16 };
    SDL_Rect palettedest = { FIELD_EDITOR_PALETTE_X, FIELD_EDITOR_PALETTE_Y, 16, 16 };

    SDL_Rect src = { 0, 0, 32, 32 };
    SDL_Rect dest = { 0, 0, 32, 32 };

    float lt = q->p1->speeds->lock;
    float l = q->p1counters->lock;
    char r = 255 - (char)(80 * l / lt);
    Uint32 rgba = (r * 0x1000000) + (r * 0x10000) + (r * 0x100) + 0xFF;

    if(YTOROW(q->p1->y) != q->locking_row)
    {
        rgba = RGBA_DEFAULT;
    }

    int i = 0;
    int x = q->field_x;
    int y = q->field_y;
    int piece_x = x + ((q->state_flags & GAMESTATE_BIGMODE ? 32 : 16) * q->p1->x);
    int piece_y = y + 16 + ((q->state_flags & GAMESTATE_BIGMODE ? 32 : 16) * (YTOROW(q->p1->y) - QRS_FIELD_H + 20));

    std::vector<int> old_piece_xs(q->p1->num_olds);
    std::vector<int> old_piece_ys(q->p1->num_olds);

    for(int j = 0; j < q->p1->num_olds; j++)
    {
        old_piece_xs[j] = x + ((q->state_flags & GAMESTATE_BIGMODE ? 32 : 16) * q->p1->old_xs[j]);
        old_piece_ys[j] = y + 16 + ((q->state_flags & GAMESTATE_BIGMODE ? 32 : 16) * (YTOROW(q->p1->old_ys[j]) - QRS_FIELD_H + 20));
    }

    if((q->state_flags & GAMESTATE_BIGMODE) && (q->game_type != Shiro::GameType::SIMULATE_QRS))
    {
        piece_x += 16;
        for(int j = 0; j < q->p1->num_olds; j++)
        {
            old_piece_xs[j] += 16;
        }
    }

    SDL_Rect labg_src = { 401, 0, 111 - 32, 64 };
    SDL_Rect labg_dest = { 264 - 48 + 4 + x, 312 - 32 + y, 111 - 32, 64};

    int preview1_x = x + 5 * 16;
    int preview2_x = q->tetromino_only ? x + 20 * 8 : x + 21 * 8;
    int preview3_x = q->tetromino_only ? x + 24 * 8 + 12 : x + 27 * 8;
    int preview4_x = q->tetromino_only ? x + 28 * 8 + 24 : x + 33 * 8;
    int preview1_y = y - 3 * 16 - (q->game_type != Shiro::GameType::SIMULATE_QRS && !q->pracdata ? 4 : 0);
    int preview2_y = y - 2 * 8 - (q->game_type != Shiro::GameType::SIMULATE_QRS && !q->pracdata ? 4 : 0);
    int preview3_y = y - 2 * 8 - (q->game_type != Shiro::GameType::SIMULATE_QRS && !q->pracdata ? 4 : 0);
    int preview4_y = y - 2 * 8 - (q->game_type != Shiro::GameType::SIMULATE_QRS && !q->pracdata ? 4 : 0);
    int hold_x = preview1_x - 8 * 6;
    int hold_y = preview2_y - 12;

    if(q->mode_type == MODE_PENTOMINO)
    {
        preview2_x -= 8;
        preview3_x -= 14;
        preview4_x -= 20;
    }

    int y_bkp = 0;
    int s_bkp = 0;

#if 0
    double mspf = 1000.0 * (1.0 / cs->fps);
    int cpu_time_percentage = (int)(100.0 * ((mspf - cs->avg_sleep_ms_recent) / mspf));
#endif

    std::string text_level = "LEVEL";
    std::string level = strtools::format("%d", q->level);
    std::string next = "NEXT";
    std::string next_name;
    if(q->previews.size() > 0)
    {
        next_name = get_qrspiece_name(q->previews[0].qrsID);
    }

    // string grade_text = get_grade_name(q->grade);
    std::string score_text = strtools::format("%d", q->score);

    std::string undo = "UNDO";
    std::string redo = "REDO";
    // string columns = "0123456789AB";
#if 0
    string avg_sleep_ms = strtools::format("%LF", cs->avg_sleep_ms_recent);
    string ctp_str = strtools::format("%d%%", cpu_time_percentage);
    string ctp_overload_str;
#endif

    std::string undo_len;
    std::string redo_len;

    struct text_formatting fmt = {
        RGBA_DEFAULT,
        RGBA_OUTLINE_DEFAULT,
        true,
        false,
        1.0,
        1.0,
        ALIGN_LEFT,
        0
    };

    // columns_adj->data = columns->data + (QRS_FIELD_W - q->field_w)/2;
    // columns_adj->slen = q->field_w;

    if(q->state_flags & GAMESTATE_FADING)
        drawqrsfield_flags |= DRAWFIELD_NO_OUTLINE;

    if(q->state_flags & GAMESTATE_INVISIBLE)
        drawqrsfield_flags |= DRAWFIELD_INVISIBLE;

    if(q->pracdata)
    {
        if(q->pracdata->paused == QRS_FIELD_EDIT)
        {
            gfx_drawqrsfield(cs, &q->pracdata->usr_field, MODE_PENTOMINO, DRAWFIELD_GRID | DRAWFIELD_NO_OUTLINE, x, y);
            if(q->pracdata->field_selection)
                gfx_drawfield_selection(g, q->pracdata);

            if(q->pracdata->usr_field_undo.size())
            {
                undo_len = strtools::format("%d", q->pracdata->usr_field_undo.size());

                gfx_drawtext(cs, undo, QRS_FIELD_X + 32, QRS_FIELD_Y + 23 * 16, monofont_square, NULL);
                gfx_drawtext(cs, undo_len, QRS_FIELD_X + 32, QRS_FIELD_Y + 24 * 16, monofont_square, NULL);

                src.x = 14 * 16;
                src.y = 64;
                dest.x = QRS_FIELD_X;
                dest.y = QRS_FIELD_Y + 23 * 16;

                SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);
            }

            if(q->pracdata->usr_field_redo.size())
            {
                redo_len = strtools::format("%d", q->pracdata->usr_field_redo.size());

                gfx_drawtext(cs, redo, QRS_FIELD_X + 9 * 16, QRS_FIELD_Y + 23 * 16, monofont_square, NULL);
                gfx_drawtext(cs, redo_len, QRS_FIELD_X + 13 * 16 - 16 * (int)redo_len.size(), QRS_FIELD_Y + 24 * 16, monofont_square, NULL);

                src.x = 16 * 16;
                src.y = 64;
                dest.x = QRS_FIELD_X + 13 * 16;
                dest.y = QRS_FIELD_Y + 23 * 16;

                SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);
            }

            if(q->pracdata->usr_seq_len)
            {
                if(qs_get_usrseq_elem(q->pracdata, 0) == QRS_I4 || qs_get_usrseq_elem(q->pracdata, 0) == QRS_I)
                    drawpiece_next1_flags = DRAWPIECE_PREVIEW | DRAWPIECE_IPREVIEW;
                else
                    drawpiece_next1_flags = DRAWPIECE_PREVIEW;

                // gfx_drawtext(cs, next, 48 - 32 + QRS_FIELD_X, 26, 0, 0xFFFFFF8C, 0x0000008C);

                if(q->num_previews > 0)
                    gfx_drawpiece(cs, g->field, x, y, q->previews[0], drawpiece_next1_flags, Shiro::Orientation::FLAT, preview1_x, preview1_y, RGBA_DEFAULT);
                if(q->num_previews > 1)
                    gfx_drawpiece(cs, g->field, x, y, q->previews[1], DRAWPIECE_PREVIEW | DRAWPIECE_SMALL, Shiro::Orientation::FLAT, preview2_x, preview2_y, RGBA_DEFAULT);
                if(q->num_previews > 2)
                    gfx_drawpiece(cs, g->field, x, y, q->previews[2], DRAWPIECE_PREVIEW | DRAWPIECE_SMALL, Shiro::Orientation::FLAT, preview3_x, preview3_y, RGBA_DEFAULT);
                if(q->num_previews > 3)
                    gfx_drawpiece(cs, g->field, x, y, q->previews[3], DRAWPIECE_PREVIEW | DRAWPIECE_SMALL, Shiro::Orientation::FLAT, preview4_x, preview4_y, RGBA_DEFAULT);
            }

            for(i = 0; i < 18; i++)
            {
                if(i < 3 || i == 4 || i == 5 || i > 10)
                    continue;

                palettesrc.x = i * 16;
                SDL_RenderCopy(cs->screen.renderer, tets_dark_qs, &palettesrc, &palettedest);
                if(q->pracdata->palette_selection - 1 == i)
                {
                    palettesrc.x = 31 * 16;
                    SDL_SetTextureAlphaMod(tets_dark_qs, 140);
                    SDL_RenderCopy(cs->screen.renderer, tets_dark_qs, &palettesrc, &palettedest);
                    SDL_SetTextureAlphaMod(tets_dark_qs, 255);
                }
                palettedest.y += 16;
            }

            palettedest.y += 16;

            for(i = 18; i < 26; i++)
            {
                palettesrc.x = i * 16;
                SDL_RenderCopy(cs->screen.renderer, tets_dark_qs, &palettesrc, &palettedest);
                if(q->pracdata->palette_selection - 1 == i || (i == 25 && q->pracdata->palette_selection == -5))
                {
                    palettesrc.x = 31 * 16;
                    SDL_SetTextureAlphaMod(tets_dark_qs, 140);
                    SDL_RenderCopy(cs->screen.renderer, tets_dark_qs, &palettesrc, &palettedest);
                    SDL_SetTextureAlphaMod(tets_dark_qs, 255);
                }
                palettedest.y += 16;
            }

            palettesrc.x = 30 * 16;
            SDL_RenderCopy(cs->screen.renderer, tets_dark_qs, &palettesrc, &palettedest);
            if(q->pracdata->palette_selection == QRS_PIECE_BRACKETS)
            {
                palettesrc.x = 31 * 16;
                SDL_SetTextureAlphaMod(tets_dark_qs, 140);
                SDL_RenderCopy(cs->screen.renderer, tets_dark_qs, &palettesrc, &palettedest);
                SDL_SetTextureAlphaMod(tets_dark_qs, 255);
            }

            palettedest.y += 16;
            palettesrc.x = 32 * 16;
            SDL_RenderCopy(cs->screen.renderer, tets_dark_qs, &palettesrc, &palettedest);
            if(q->pracdata->palette_selection == QRS_PIECE_GEM)
            {
                palettesrc.x = 31 * 16;
                SDL_SetTextureAlphaMod(tets_dark_qs, 140);
                SDL_RenderCopy(cs->screen.renderer, tets_dark_qs, &palettesrc, &palettedest);
                SDL_SetTextureAlphaMod(tets_dark_qs, 255);
            }
        }
        else
        {
            if(q->pracdata->invisible)
            {
                drawqrsfield_flags |= DRAWFIELD_INVISIBLE;
            }

            gfx_drawqrsfield(cs, g->field, MODE_PENTOMINO, drawqrsfield_flags, x, y);
            gfx_drawtimer(cs, &q->timer, x + 32, RGBA_DEFAULT);
            gfx_drawkeys(cs, &cs->keys, q->field_x + (18 * 16), 27 * 16, RGBA_DEFAULT);

            goto active_game_drawing;
        }
    }
    else
    {
        if(q->mode_type != MODE_PENTOMINO)
            gfx_drawqrsfield(cs, g->field, q->mode_type, drawqrsfield_flags | TEN_W_TETRION/* | DRAWFIELD_JEWELED*/, x, y);
        else
            gfx_drawqrsfield(cs, g->field, q->mode_type, drawqrsfield_flags, x, y);

        if(q->p1->speeds->grav >= 20 * 256)
        {
            if(cs->frames % 4 == 3)
                gfx_drawtimer(cs, &q->timer, x + 32, RGBA_DEFAULT);
            else
                gfx_drawtimer(cs, &q->timer, x + 32, 0xE0E030FF);
        }
        else
            gfx_drawtimer(cs, &q->timer, x + 32, RGBA_DEFAULT);

        if(cs->displayMode == Shiro::DisplayMode::DETAILED)
        {
            gfx_drawkeys(cs, &cs->keys, q->field_x + (14 * 16), 27 * 16, RGBA_DEFAULT);

            std::string secTimeStr;
            struct text_formatting secTimeFmt = {
                RGBA_DEFAULT,
                0x000000A0,
                false,
                false,
                2.0,
                1.0,
                ALIGN_RIGHT,
                0
            };

            long cumulativeTime = 0;

            int secX = 356;
            int secY = 48;

            SDL_Rect secTimeBGRect = { secX, secY, 640 - secX + 6, 24 };

            Uint8 r_;
            Uint8 g_;
            Uint8 b_;
            Uint8 a_;
            SDL_GetRenderDrawColor(cs->screen.renderer, &r_, &g_, &b_, &a_);
            SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 0x80);

            for(int sec = 0; sec < MAX_SECTIONS; sec++)
            {
                if(q->section_times[sec] != -1)
                {
                    cumulativeTime += q->section_times[sec];

                    if(sec < MAX_SECTIONS - 1 && q->section_times[sec + 1] != -1)
                    {
                        secTimeBGRect.h = 24;
                    }
                    else
                    {
                        secTimeBGRect.h = 32;
                    }

                    secTimeBGRect.y = secY;

                    SDL_RenderFillRect(cs->screen.renderer, &secTimeBGRect);

                    int minutes = q->section_times[sec] / (60*60);
                    int seconds = (q->section_times[sec] / 60) % 60;
                    int centiseconds = (int)((double)(q->section_times[sec] % 60) * 100.0 / 60.0);

                    int cuMinutes = cumulativeTime / (60*60);
                    int cuSeconds = (cumulativeTime / 60) % 60;
                    int cuCentiseconds = (int)((double)(cumulativeTime % 60) * 100.0 / 60.0);

                    int textX = 634;

                    secTimeStr = strtools::format("%d:%02d:%02d", cuMinutes, cuSeconds, cuCentiseconds);
                    secTimeFmt.rgba = 0x909090FF;
                    gfx_drawtext(cs, secTimeStr, textX, secY, monofont_fixedsys, &secTimeFmt);
                    secTimeFmt.rgba = RGBA_DEFAULT;
                    textX -= 7*16 + 8;

                    if(minutes == 0)
                    {
                        secTimeStr = strtools::format("   %02d:%02d", seconds, centiseconds);
                        gfx_drawtext(cs, secTimeStr, textX, secY, monofont_fixedsys, &secTimeFmt);
                    }
                    else
                    {
                        secTimeStr = strtools::format(" %d:%02d:%02d", minutes, seconds, centiseconds);
                        gfx_drawtext(cs, secTimeStr, textX, secY, monofont_fixedsys, &secTimeFmt);
                    }

                    textX -= 7*16 + 8;
                    secTimeStr = strtools::format("%d", sec+1);
                    secTimeFmt.rgba = 0x8080B0FF;
                    gfx_drawtext(cs, secTimeStr, textX, secY, monofont_fixedsys, &secTimeFmt);
                }

                secY += 24;
            }

            SDL_SetRenderDrawColor(cs->screen.renderer, r_, g_, b_, a_);
        }

        // uncomment this for DDR-esque input display :3
        /*if(q->playback) {
            for(i = 1; i < 24 && i < (q->replay->len - q->playback_index)/3; i++) {
                gfx_drawkeys(cs, &q->replay->inputs[3*i+q->playback_index], 22*16, (25-i)*16, RGBA_DEFAULT);
            }
        }*/

        SDL_RenderCopy(cs->screen.renderer, font, &labg_src, &labg_dest);
        labg_src.x = 512 - 16;
        labg_src.w = 16;
        labg_dest.w = 16;
        labg_dest.x += (111 - 32);
        SDL_RenderCopy(cs->screen.renderer, font, &labg_src, &labg_dest);

        labg_dest.y -= 5 * 16;
        labg_src.w = 111;
        labg_src.x = 401;
        labg_dest.w = 111;
        labg_dest.x -= 111 - 32;
        SDL_RenderCopy(cs->screen.renderer, font, &labg_src, &labg_dest);

        if(q->p1->speeds->grav >= 20 * 256)
        {
            if(cs->frames % 4 == 3)
            {
                gfx_drawtext(cs, text_level, x + 14 * 16 + 4, y + 18 * 16, monofont_square, NULL);
            }
            else
            {
                fmt.rgba = 0xE0E030FF;
                gfx_drawtext(cs, text_level, x + 14 * 16 + 4, y + 18 * 16, monofont_square, &fmt);
            }

            fmt.rgba = 0xFFC020FF;
            gfx_drawtext(cs, level, x + 14 * 16 + 4, y + 20 * 16, monofont_square, &fmt);
        }
        else
        {
            gfx_drawtext(cs, text_level, x + 14 * 16 + 4, y + 18 * 16, monofont_square, NULL);
            fmt.rgba = 0xFF7070FF;
            gfx_drawtext(cs, level, x + 14 * 16 + 4, y + 20 * 16, monofont_square, &fmt);
        }

        if((q->grade & 0xff) != NO_GRADE)
        {
            int gradeWithoutFlags = q->grade & 0xff;

            SDL_Rect grade_src = { 0, 390, 64, 64 };
            SDL_Rect grade_dest = { x + 14 * 16, y + 32, 64, 64 };
            float size_multiplier = 1.0;

            // draw a shadowy square behind the grade
            // SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);

            grade_src.w = 64;
            grade_src.h = 64;

            if(gradeWithoutFlags < GRADE_S1)
            {
                grade_src.w = 32;
                grade_src.h = 32;
                grade_dest.w = 32;
                grade_dest.h = 32;
                grade_dest.x += 16;
                grade_dest.y += 16;
            }

            if(g->frame_counter - q->last_gradeup_timestamp < 20)
            {
                size_multiplier = 1.8 - 0.04 * (g->frame_counter - q->last_gradeup_timestamp);
                grade_dest.x -= ((size_multiplier - 1.0) / 2) * grade_dest.w;
                grade_dest.y -= ((size_multiplier - 1.0) / 2) * grade_dest.h;

                grade_dest.w *= size_multiplier;
                grade_dest.h *= size_multiplier;
            }

            grade_src.y = 127;

            if(gradeWithoutFlags < GRADE_S1)
            {
                grade_dest.x += 3;
            }

            grade_dest.y += 3;

            if(q->p1->speeds->grav >= 20 * 256)
            {
                if(cs->frames % 4 != 3)
                {
                    SDL_SetTextureColorMod(font, 0xE0, 0xE0, 0x30);
                }
            }

            switch(gradeWithoutFlags)
            {
                case GRADE_1:
                case GRADE_2:
                case GRADE_3:
                case GRADE_4:
                case GRADE_5:
                case GRADE_6:
                case GRADE_7:
                case GRADE_8:
                case GRADE_9:
                    grade_src.x += 2 * 64 + 32 * (GRADE_1 - gradeWithoutFlags);
                    grade_src.w = 32;
                    grade_src.h = 32;

                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    break;

                    /*case GRADE_S9:
                        SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                        grade_src.y += 32;
                        grade_src.x += 128 + 8*32;

                        grade_dest.x += 47*size_multiplier;
                        grade_dest.y += 30*size_multiplier;
                        grade_src.w = 32;
                        grade_src.h = 32;
                        grade_dest.w = 32*size_multiplier;
                        grade_dest.h = 32*size_multiplier;
                        SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                        break;*/

                case GRADE_S1:
                case GRADE_S2:
                case GRADE_S3:
                case GRADE_S4:
                case GRADE_S5:
                case GRADE_S6:
                case GRADE_S7:
                case GRADE_S8:
                case GRADE_S9:
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    grade_src.y += 32;
                    grade_src.x = 128 + 32 * (gradeWithoutFlags - GRADE_S1);

                    grade_dest.x += 47 * size_multiplier;
                    grade_dest.y += 30 * size_multiplier;
                    grade_src.w = 32;
                    grade_src.h = 32;
                    grade_dest.w = 32 * size_multiplier;
                    grade_dest.h = 32 * size_multiplier;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    break;

                case GRADE_S10:
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    grade_src.y += 32;
                    grade_src.x = 128;

                    grade_dest.x += 47 * size_multiplier;
                    grade_dest.y += 30 * size_multiplier;
                    grade_src.w = 32;
                    grade_src.h = 32;
                    grade_dest.w = 32 * size_multiplier;
                    grade_dest.h = 32 * size_multiplier;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    grade_src.x = 128 + 9 * 32;
                    grade_dest.x += 20 * size_multiplier;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    break;

                case GRADE_S11:
                case GRADE_S12:
                case GRADE_S13:
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    grade_src.y += 32;
                    grade_src.x = 128;

                    grade_dest.x += 47 * size_multiplier;
                    grade_dest.y += 30 * size_multiplier;
                    grade_src.w = 32;
                    grade_src.h = 32;
                    grade_dest.w = 32 * size_multiplier;
                    grade_dest.h = 32 * size_multiplier;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    grade_src.x += 32 * (gradeWithoutFlags - GRADE_S11);
                    grade_dest.x += 20 * size_multiplier;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    break;

                case GRADE_M1:
                case GRADE_M2:
                case GRADE_M3:
                case GRADE_M4:
                case GRADE_M5:
                case GRADE_M6:
                case GRADE_M7:
                case GRADE_M8:
                case GRADE_M9:
                    grade_src.x += 64;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    grade_src.y += 32;
                    grade_src.x = 128 + 32 * (gradeWithoutFlags - GRADE_M1);

                    grade_dest.x += 42 * size_multiplier;
                    grade_dest.y += 30 * size_multiplier;
                    grade_src.w = 32;
                    grade_src.h = 32;
                    grade_dest.w = 32 * size_multiplier;
                    grade_dest.h = 32 * size_multiplier;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    break;

                case GRADE_M:
                    grade_src.x = 192;
                    grade_src.y += 64;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);
                    break;

                case GRADE_MK:
                case GRADE_MV:
                case GRADE_MO:
                case GRADE_MM:
                    grade_src.x = 192;
                    grade_src.y += 64;

                    grade_dest.x -= 14 * size_multiplier;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);

                    grade_src.x = 64 * (gradeWithoutFlags - GRADE_MK);
                    grade_dest.x += 38 * size_multiplier;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);

                    break;

                case GRADE_GM:
                    grade_src.x = 256;
                    grade_src.y += 64;

                    grade_dest.x -= 14 * size_multiplier;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);

                    grade_src.x = 192;
                    grade_dest.x += 38 * size_multiplier;
                    SDL_RenderCopy(cs->screen.renderer, font, &grade_src, &grade_dest);

                    break;

                default:
                    break;
            }

            SDL_SetTextureColorMod(font, 255, 255, 255);
        }

        if(q->p1->speeds->grav >= 20 * 256)
        {
            if(cs->frames % 4 != 3)
            {
                fmt.rgba = 0xE0E030FF;
            }
            else
                fmt.rgba = RGBA_DEFAULT;

            gfx_drawtext(cs, "SCORE", x + 14 * 16 + 4, y + 13 * 16, monofont_square, &fmt);
            fmt.rgba = 0x6060FFFF;
            gfx_drawtext(cs, score_text, x + 14 * 16 + 4, y + 15 * 16, monofont_square, &fmt);
        }
        else
        {
            fmt.rgba = RGBA_DEFAULT;
            gfx_drawtext(cs, "SCORE", x + 14 * 16 + 4, y + 13 * 16, monofont_square, &fmt);
            fmt.rgba = 0x20FF20FF;
            gfx_drawtext(cs, score_text, x + 14 * 16 + 4, y + 15 * 16, monofont_square, &fmt);
        }

    active_game_drawing:
        if(q->mode_type == MODE_PENTOMINO)
        {
            fmt.rgba = RGBA_DEFAULT;
            fmt.outlined = false;

            gfx_drawtext(cs, next, q->field_x + 12, 13, monofont_small, &fmt);
            gfx_drawtext(cs, next_name, q->field_x + 12, 25, monofont_small, &fmt);

            if(histrand_get_difficulty(q->randomizer) > 0.0)
            {
                labg_src.x = 432;
                labg_src.y = 64;
                labg_src.w = 80;
                labg_src.h = 16;

                labg_dest.x = x + 14 * 16 - 3;
                labg_dest.y = y + 22 * 16;
                labg_dest.w = 80;
                labg_dest.h = 16;
                SDL_RenderCopy(cs->screen.renderer, font, &labg_src, &labg_dest);

                gfx_drawtext(cs, "RANK", x + 14 * 16 + 1, y + 22 * 16, monofont_fixedsys, &fmt);

                fmt.rgba = 0xFFA0A0FF;
                fmt.align = ALIGN_RIGHT;
                gfx_drawtext(cs, strtools::format("%.1f", histrand_get_difficulty(q->randomizer)), x + 19 * 16 - 6, y + 22 * 16, monofont_fixedsys, &fmt);
                fmt.align = ALIGN_LEFT;
            }
        }

        if(q->num_previews > 0)
            gfx_drawpiece(cs, g->field, x, y, q->previews[0], drawpiece_flags | drawpiece_next1_flags, Shiro::Orientation::FLAT, preview1_x, preview1_y, RGBA_DEFAULT);
        if(q->num_previews > 1)
            gfx_drawpiece(
                cs, g->field, x, y, q->previews[1], drawpiece_flags | DRAWPIECE_PREVIEW | DRAWPIECE_SMALL, Shiro::Orientation::FLAT, preview2_x, preview2_y, RGBA_DEFAULT);
        if(q->num_previews > 2)
            gfx_drawpiece(
                cs, g->field, x, y, q->previews[2], drawpiece_flags | DRAWPIECE_PREVIEW | DRAWPIECE_SMALL, Shiro::Orientation::FLAT, preview3_x, preview3_y, RGBA_DEFAULT);
        if(q->num_previews > 3)
            gfx_drawpiece(
                cs, g->field, x, y, q->previews[3], drawpiece_flags | DRAWPIECE_PREVIEW | DRAWPIECE_SMALL, Shiro::Orientation::FLAT, preview4_x, preview4_y, RGBA_DEFAULT);

        if(q->hold)
        {
            gfx_drawpiece(cs,
                          g->field,
                          x,
                          y,
                          *q->hold,
                          drawpiece_flags | DRAWPIECE_PREVIEW | DRAWPIECE_SMALL | (q->p1->state & PSUSEDHOLD ? DRAWPIECE_LOCKFLASH : 0),
                          Shiro::Orientation::FLAT,
                          hold_x,
                          hold_y,
                          RGBA_DEFAULT);
        }

        if((q->p1->state & (PSFALL | PSLOCK)) && !(q->p1->state & PSPRELOCKED) && pd_current)
        {
            if(!(pd_current->flags & Shiro::PDBRACKETS))
            {
                y_bkp = q->p1->y;
                s_bkp = q->p1->state;
                qrs_fall(g, q->p1, 28 * 256);
                piece_y = y + 16 + ((q->state_flags & GAMESTATE_BIGMODE ? 32 : 16) * (YTOROW(q->p1->y) - QRS_FIELD_H + 20));

                switch(q->mode_type)
                {
                    case MODE_PENTOMINO:
                        if(q->level < 300)
                            gfx_drawpiece(cs, g->field, x, y, *pd_current, drawpiece_flags, q->p1->orient, piece_x, piece_y, 0xFFFFFF60);

                        break;

                    default:
                        if(q->level < 100)
                            gfx_drawpiece(cs, g->field, x, y, *pd_current, drawpiece_flags, q->p1->orient, piece_x, piece_y, 0xFFFFFF60);

                        break;
                }

                q->p1->y = y_bkp;
                q->p1->state = s_bkp;
                piece_y = y + 16 + ((q->state_flags & GAMESTATE_BIGMODE ? 32 : 16) * (YTOROW(q->p1->y) - QRS_FIELD_H + 20));
            }

            if(pd_current->flags & Shiro::PDBRACKETS)
            {
                rgba = RGBA_DEFAULT;
            }

            if(cs->motionBlur)
            {
                for(int j = q->p1->num_olds - 1; j >= 0; j--)
                {
                    gfx_drawpiece(cs, g->field, x, y, *pd_current, drawpiece_flags, q->p1->orient, old_piece_xs[j], old_piece_ys[j], 0xFFFFFF00 + (0xC0 / (j + 1)) );
//                     if (g->frame_counter % 60 == 0) {
//                        std::cerr << "Old piece x # " << j << ": " << q->p1->old_xs[j] << std::endl;
//                     }
                }
            }

            gfx_drawpiece(cs, g->field, x, y, *pd_current, drawpiece_flags, q->p1->orient, piece_x, piece_y, rgba);
        }
        else if(q->p1->state & (PSLOCKFLASH1 | PSLOCKFLASH2) && !(q->state_flags & GAMESTATE_BRACKETS))
            gfx_drawpiece(cs, g->field, x, y, *pd_current, drawpiece_flags | DRAWPIECE_LOCKFLASH, q->p1->orient, piece_x, piece_y, RGBA_DEFAULT);
        else if(q->p1->state & PSPRELOCKED)
        {
            if(q->state_flags & GAMESTATE_BRACKETS)
                gfx_drawpiece(cs, g->field, x, y, *pd_current, drawpiece_flags, q->p1->orient, piece_x, piece_y, RGBA_DEFAULT);
            else
                gfx_drawpiece(cs, g->field, x, y, *pd_current, drawpiece_flags, q->p1->orient, piece_x, piece_y, 0x404040FF);
        }
    }

    // gfx_drawtime(cs, 123456, 500, 400, RGBA_DEFAULT);

    /*if(q->pracdata) {
        if(q->pracdata->paused == QRS_FIELD_EDIT)
            gfx_drawtext(cs, columns_adj, QRS_FIELD_X + 16 + 8*(QRS_FIELD_W - q->field_w), QRS_FIELD_Y + 16, 0, 0xFFFFFFB0, 0x000000B0);
    }*/

    gfx_drawqsmedals(g);

    fmt.shadow = true;
    fmt.outlined = false;
    fmt.rgba = 0x7070D0FF;
    // gfx_drawtext(cs, ctp_str, 640 - 16 + 16 * (1 - ctp_str->slen), 2, monofont_square, &fmt);

#if 0
    if(cs->recent_frame_overload >= 0)
    {
        cpu_time_percentage = (int)(100.0 * ((mspf - cs->avg_sleep_ms_recent_array[cs->recent_frame_overload]) / mspf));
        ctp_overload_str = strtools::format("%d%%", cpu_time_percentage);

        fmt.rgba = 0xB00000FF;
        gfx_drawtext(cs, ctp_overload_str, 640 - 16 + 16 * (1 - ctp_overload_str.size()), 2, monofont_square, &fmt);
    }
#endif

    return 0;
}

int gfx_qs_lineclear(game_t *g, int row)
{
    qrsdata *q = (qrsdata *)g->data;
    if(q->pracdata)
    {
        if(q->pracdata->brackets)
            return 0;
    }

    int i = 0;
    int mod = 0;
    int c = 0;
    gfx_image* frames = g->origin->assets->animation_lineclear;

    for(i = (QRS_FIELD_W - q->field_w) / 2; i < (QRS_FIELD_W + q->field_w) / 2; i += 2)
    {
        c = g->field->getCell(i, row);
        if((c == 0) || (c & QRS_PIECE_BRACKETS))
        {
            continue;
        }

        if(c > 0)
        {
            mod = piece_colors[(c & 0xFF) - 1] * 0x100 + 0xFF;
        }
        else
        {
            mod = piece_colors[25] * 0x100 + 0xFF;
        }

        if(row % 2)
        {
            Shiro::AnimationEntity::push(g->origin->gfx,
                frames,
                static_cast<size_t>(Shiro::GfxLayer::animations),
                q->field_x + (i * 16),
                q->field_y + 16 + (16 * (row - QRS_FIELD_H + 20)),
                5,
                3,
                mod
            );
        }
        else
        {
            Shiro::AnimationEntity::push(g->origin->gfx,
                frames,
                static_cast<size_t>(Shiro::GfxLayer::animations),
                q->field_x + (i * 16) + 16,
                q->field_y + 16 + (16 * (row - QRS_FIELD_H + 20)),
                5,
                3,
                mod
            );
        }
    }

    return 0;
}

int gfx_drawqsmedals(game_t *g)
{
    if(!g)
        return -1;

    qrsdata *q = (qrsdata *)g->data;
    SDL_Rect dest = { 228 + q->field_x, 150, 40, 20 };
    SDL_Rect src = { 20, 0, 20, 10 };
    SDL_Texture *medals = g->origin->assets->medals.tex;
    bool medal = true;

    float size_multiplier = 1.0;

    switch(q->medal_re)
    {
        case BRONZE:
            src.y = 0;
            break;
        case SILVER:
            src.y = 10;
            break;
        case GOLD:
            src.y = 20;
            break;
        case PLATINUM:
            src.y = 30;
            break;
        default:
            medal = false;
            break;
    }

    if(medal)
    {
        if(g->frame_counter - q->last_medal_re_timestamp < 20)
        {
            size_multiplier = 1.8 - 0.04 * (g->frame_counter - q->last_medal_re_timestamp);

            SDL_Rect dest_ = { dest.x, dest.y, 40, 20 };

            dest_.w *= size_multiplier;
            dest_.h *= size_multiplier;
            dest_.x -= ((size_multiplier - 1.0) / 2) * 40;
            dest_.y -= ((size_multiplier - 1.0) / 2) * 20;

            SDL_RenderCopy(g->origin->screen.renderer, medals, &src, &dest_);
        }
        else
            SDL_RenderCopy(g->origin->screen.renderer, medals, &src, &dest);
    }

    dest.y += 24;
    medal = true;
    src.x = 40;

    switch(q->medal_sk)
    {
        case BRONZE:
            src.y = 0;
            break;
        case SILVER:
            src.y = 10;
            break;
        case GOLD:
            src.y = 20;
            break;
        case PLATINUM:
            src.y = 30;
            break;
        default:
            medal = false;
            break;
    }

    if(medal)
    {
        if(g->frame_counter - q->last_medal_sk_timestamp < 20)
        {
            size_multiplier = 1.8 - 0.04 * (g->frame_counter - q->last_medal_sk_timestamp);

            SDL_Rect dest_ = { dest.x, dest.y, 40, 20 };

            dest_.w *= size_multiplier;
            dest_.h *= size_multiplier;
            dest_.x -= ((size_multiplier - 1.0) / 2) * 40;
            dest_.y -= ((size_multiplier - 1.0) / 2) * 20;

            SDL_RenderCopy(g->origin->screen.renderer, medals, &src, &dest_);
        }
        else
            SDL_RenderCopy(g->origin->screen.renderer, medals, &src, &dest);
    }

    dest.y += 24;
    medal = true;
    src.x = 0;

    switch(q->medal_co)
    {
        case BRONZE:
            src.y = 0;
            break;
        case SILVER:
            src.y = 10;
            break;
        case GOLD:
            src.y = 20;
            break;
        case PLATINUM:
            src.y = 30;
            break;
        default:
            medal = false;
            break;
    }

    if(medal)
    {
        if(g->frame_counter - q->last_medal_co_timestamp < 20)
        {
            size_multiplier = 1.8 - 0.04 * (g->frame_counter - q->last_medal_co_timestamp);

            SDL_Rect dest_ = { dest.x, dest.y, 40, 20 };

            dest_.w *= size_multiplier;
            dest_.h *= size_multiplier;
            dest_.x -= ((size_multiplier - 1.0) / 2) * 40;
            dest_.y -= ((size_multiplier - 1.0) / 2) * 20;

            SDL_RenderCopy(g->origin->screen.renderer, medals, &src, &dest_);
        }
        else
            SDL_RenderCopy(g->origin->screen.renderer, medals, &src, &dest);
    }

    dest.y += 24;
    medal = true;
    src.x = 80;

    switch(q->medal_st)
    {
        case BRONZE:
            src.y = 0;
            break;
        case SILVER:
            src.y = 10;
            break;
        case GOLD:
            src.y = 20;
            break;
        case PLATINUM:
            src.y = 30;
            break;
        default:
            medal = false;
            break;
    }

    if(medal)
    {
        if(g->frame_counter - q->last_medal_st_timestamp < 20)
        {
            size_multiplier = 1.8 - 0.04 * (g->frame_counter - q->last_medal_st_timestamp);

            SDL_Rect dest_ = { dest.x, dest.y, 40, 20 };

            dest_.w *= size_multiplier;
            dest_.h *= size_multiplier;
            dest_.x -= ((size_multiplier - 1.0) / 2) * 40;
            dest_.y -= ((size_multiplier - 1.0) / 2) * 20;

            SDL_RenderCopy(g->origin->screen.renderer, medals, &src, &dest_);
        }
        else
            SDL_RenderCopy(g->origin->screen.renderer, medals, &src, &dest);
    }

    return 0;
}

int gfx_drawfield_selection(game_t *g, struct pracdata *d)
{
    qrsdata *q = (qrsdata *)g->data;

    SDL_Texture *tets = g->origin->assets->tets_bright_qs.tex;
    SDL_Rect src = { 31 * 16, 0, 16, 16 };
    SDL_Rect dest = { 0, 0, 16, 16 };

    int i = 0;
    int j = 0;

    int lesser_x = 0;
    int greater_x = 0;
    int lesser_y = 0;
    int greater_y = 0;

    if(d->field_selection_vertex1_x <= d->field_selection_vertex2_x)
    {
        lesser_x = d->field_selection_vertex1_x;
        greater_x = d->field_selection_vertex2_x;
    }
    else
    {
        lesser_x = d->field_selection_vertex2_x;
        greater_x = d->field_selection_vertex1_x;
    }

    if(d->field_selection_vertex1_y <= d->field_selection_vertex2_y)
    {
        lesser_y = d->field_selection_vertex1_y;
        greater_y = d->field_selection_vertex2_y;
    }
    else
    {
        lesser_y = d->field_selection_vertex2_y;
        greater_y = d->field_selection_vertex1_y;
    }

    SDL_SetTextureColorMod(tets, 170, 170, 255);
    SDL_SetTextureAlphaMod(tets, 160);

    for(i = lesser_x; i <= greater_x; i++)
    {
        for(j = lesser_y; j <= greater_y; j++)
        {
            if(i >= 0 && i < 12 && j >= 0 && j < 20)
            {
                if (d->usr_field.getCell(i, j + 2) != QRS_FIELD_W_LIMITER) {
                    dest.x = q->field_x + 16 * (i + 1);
                    dest.y = QRS_FIELD_Y + 16 * (j + 2);

                    SDL_RenderCopy(g->origin->screen.renderer, tets, &src, &dest);
                }
            }
        }
    }

    SDL_SetTextureColorMod(tets, 255, 255, 255);
    SDL_SetTextureAlphaMod(tets, 255);

    return 0;
}