#include "gfx_menu.h"
#include "CoreState.h"
#include "game_menu.h"
#include "gfx_old.h"
#include "Menu/ElementType.h"
#include "Menu/GameMultiOption.h"
#include "Menu/MultiOption.h"
#include "Menu/Option.h"
#include "Menu/GameOption.h"
#include "Menu/TextOption.h"
#include "Menu/ToggleOption.h"
#include <cstdlib>
#include <iostream>
#include "SDL.h"
int gfx_drawmenu(game_t *g)
{
    if(!g)
        return -1;
    if(!g->origin)
        return -1;

    CoreState *cs = g->origin;

    SDL_Texture *font = Shiro::ImageAsset::get(cs->assetMgr, "font").getTexture();
    SDL_Texture *font_thin = Shiro::ImageAsset::get(cs->assetMgr, "font_thin").getTexture();
    SDL_Rect src = { 0, 80, 16, 16 };
    SDL_Rect dest = { 0, 0, 16, 16 };
    SDL_Rect barsrc = { 12 * 16, 17, 2, 14 };
    SDL_Rect bardest = { 0, 0, 2, 14 };
    SDL_Rect baroutlinesrc = { 256, 0, 102, 16 };
    SDL_Rect baroutlinedest = { 0, 0, 102, 16 };

    menudata *d = (menudata *)(g->data);
    Shiro::MenuOption *m = NULL;
    Shiro::MultiOptionData *d2 = NULL;
    Shiro::GameMultiOptionData *d3 = NULL;
    Shiro::TextOptionData *d7 = NULL;
    Shiro::ToggleOptionData *d8 = NULL;

    std::string textinput_display;
    std::string page_str;

    int i = 0;
    int j = 0;
    int k = 0;
    int mod = 0;

    int initial_opt = 0;
    int final_opt = d->numopts - 1;

    text_formatting fmt;
    png_monofont *monofont = NULL;

    if(d->is_paged)
    {
        std::stringstream ss;
        ss << "PAGE " << d->page + 1 << "/" << ((d->numopts - 1) / d->page_length) + 1;
        page_str = ss.str();
        fmt = text_fmt_create(DRAWTEXT_ALIGN_RIGHT, RGBA_DEFAULT, RGBA_OUTLINE_DEFAULT);

        gfx_drawtext(cs, page_str, d->page_text_x, d->page_text_y, monofont_square, &fmt);

        initial_opt = d->page * d->page_length;
        final_opt = d->page * d->page_length + d->page_length - 1;
        if(final_opt > d->numopts - 1)
            final_opt = d->numopts - 1;
    }

    if(!d->menu.size())
        return 0;

    if(d->title != "")
    {
        if(menu_is_main(g))
        {
            SDL_Rect titlePNGdest = { 62, 30, 300, 50 };
            SDL_SetTextureColorMod(Shiro::ImageAsset::get(cs->assetMgr, "title_emboss").getTexture(), 180, 180, 255);
            //SDL_SetTextureAlphaMod(Shiro::ImageAsset::get(cs->assetMgr, "title_emboss").getTexture(), 150);
            SDL_SetTextureBlendMode(Shiro::ImageAsset::get(cs->assetMgr, "title_emboss").getTexture(), SDL_BLENDMODE_BLEND);
            SDL_SetTextureBlendMode(Shiro::ImageAsset::get(cs->assetMgr, "bg_temp").getTexture(), SDL_BLENDMODE_BLEND);
            SDL_RenderCopy(cs->screen.renderer, Shiro::ImageAsset::get(cs->assetMgr, "title_emboss").getTexture(), NULL, &titlePNGdest);
        }
        else
        {
            gfx_drawtext(cs, d->title, d->x, d->y, monofont_square, NULL);
        }
    }

    if(d->use_target_tex)
    {
        SDL_SetRenderTarget(cs->screen.renderer, d->target_tex);
    }

    for(i = initial_opt; i <= final_opt; i++)
    {
        m = &d->menu[i];

        if(d->use_target_tex && !m->render_update)
        {
            continue;
        }
        else if(d->use_target_tex && i == initial_opt)
        {
            SDL_SetRenderDrawColor(g->origin->screen.renderer, 0, 0, 0, 0);
            SDL_RenderClear(g->origin->screen.renderer);
        }

        fmt = text_fmt_create(m->label_text_flags, m->label_text_rgba, RGBA_OUTLINE_DEFAULT);
        monofont = monofont_square;

        if(m->label_text_flags & DRAWTEXT_THIN_FONT)
            monofont = monofont_thin;
        if(m->label_text_flags & DRAWTEXT_SMALL_FONT)
            monofont = monofont_small;
        if(m->label_text_flags & DRAWTEXT_TINY_FONT)
            monofont = monofont_tiny;
        if(m->label_text_flags & DRAWTEXT_FIXEDSYS_FONT)
            monofont = monofont_fixedsys;

        if(i == d->selection)
        {
            fmt.rgba = 0x9090FFFF;
            fmt.outline_rgba = 0x2020AFFF;
            fmt.shadow = true;
        }

        gfx_drawtext(cs, m->label, m->x, m->y, monofont, &fmt);

        if(m->type == Shiro::ElementType::MENU_MULTIOPT)
        {
            d2 = (Shiro::MultiOptionData *)m->data;
            fmt = text_fmt_create(m->value_text_flags, m->value_text_rgba, RGBA_OUTLINE_DEFAULT);
            monofont = monofont_square;

            if(m->value_text_flags & DRAWTEXT_THIN_FONT)
                monofont = monofont_thin;
            if(m->value_text_flags & DRAWTEXT_SMALL_FONT)
                monofont = monofont_small;
            if(m->value_text_flags & DRAWTEXT_TINY_FONT)
                monofont = monofont_tiny;
            if(m->value_text_flags & DRAWTEXT_FIXEDSYS_FONT)
                monofont = monofont_fixedsys;

            gfx_drawtext(cs, d2->labels[d2->selection], m->value_x, m->value_y, monofont, &fmt);

            if(m->value_text_flags & DRAWTEXT_VALUE_BAR)
            {
                barsrc.x = 14 * 16;
                bardest.x = m->value_x;
                baroutlinedest.x = bardest.x;
                bardest.y = m->value_y + 1;
                baroutlinedest.y = m->value_y;
                SDL_RenderCopy(cs->screen.renderer, font, &baroutlinesrc, &baroutlinedest);

                if(d2->selection > 0)
                {
                    barsrc.x += 1;
                    bardest.x += 1;
                    barsrc.w = 1;
                    bardest.w = 1;
                    for(j = 0; j < d2->selection; j++)
                    {
                        mod = (200 * (85 - j)) / 100;
                        if(mod < 0)
                            mod = 0;

                        if((i % 3) == 1)
                            SDL_SetTextureColorMod(font, 255, mod, mod);
                        else if((i % 3) == 2)
                            SDL_SetTextureColorMod(font, mod, 255, mod);
                        else if((i % 3) == 0)
                            SDL_SetTextureColorMod(font, mod, mod, 255);

                        SDL_RenderCopy(cs->screen.renderer, font, &barsrc, &bardest);
                        bardest.x += 1;
                    }

                    SDL_SetTextureColorMod(font, 255, 255, 255);
                }
            }
        }

        if(m->type == Shiro::ElementType::MENU_GAME_MULTIOPT)
        {
            d3 = (Shiro::GameMultiOptionData *)m->data;
            if(d3->labels.size())
            {
                if(d3->labels[d3->selection] != "")
                {
                    fmt = text_fmt_create(m->value_text_flags, m->value_text_rgba, RGBA_OUTLINE_DEFAULT);
                    monofont = monofont_square;

                    if(m->value_text_flags & DRAWTEXT_THIN_FONT)
                        monofont = monofont_thin;
                    if(m->value_text_flags & DRAWTEXT_SMALL_FONT)
                        monofont = monofont_small;
                    if(m->value_text_flags & DRAWTEXT_TINY_FONT)
                        monofont = monofont_tiny;
                    if(m->value_text_flags & DRAWTEXT_FIXEDSYS_FONT)
                        monofont = monofont_fixedsys;

                    gfx_drawtext(cs, d3->labels[d3->selection], m->value_x, m->value_y, monofont, &fmt);
                }
            }
        }

        if(m->type == Shiro::ElementType::MENU_TEXTINPUT)
        {
            d7 = (Shiro::TextOptionData *)m->data;

            if(d7->text != "")
            {
                textinput_display = d7->text.substr(d7->leftmost_position);
                if (textinput_display != "" && textinput_display.size() > decltype(textinput_display)::size_type(d7->visible_chars))
                    textinput_display.resize(d7->visible_chars);
                    //btrunc(textinput_display, d7->visible_chars);

                if(d7->selection)
                {
                    SDL_SetTextureColorMod(font, 255, 255, 255);
                    SDL_SetTextureAlphaMod(font, 255);
                    src.x = 17 * 16 - 1;
                    src.y = 32 - 1;
                    src.h = 18;
                    src.w = (m->value_text_flags & DRAWTEXT_THIN_FONT ? 15 : 18);
                    dest.w = src.w;
                    dest.h = 18;
                    dest.y = m->value_y + (m->value_text_flags & DRAWTEXT_THIN_FONT ? 1 : 0);
                    for(k = 0; k < (int(d7->text.size()) - d7->leftmost_position) && k < d7->visible_chars; k++)
                    {
                        dest.x = m->value_x + (m->value_text_flags & DRAWTEXT_THIN_FONT ? 13 : 16) * (k)-1;

                        if(SDL_RenderCopy(cs->screen.renderer, font, &src, &dest)) {
                            std::cerr << SDL_GetError() << std::endl;
                        }
                    }

                    src.h = 16;
                    src.w = 16;
                    dest.w = 16;
                    dest.h = 16;
                }

                if(d7->leftmost_position > 0)
                {
                    src.x = 64;
                    src.y = 80;
                    if(m->value_text_flags & DRAWTEXT_ALIGN_RIGHT)
                    {
                        dest.x = m->value_x - ((m->value_text_flags & DRAWTEXT_THIN_FONT ? 13 : 16) * d7->visible_chars) - 16;
                    }
                    else
                    {
                        dest.x = m->value_x - 16;
                    }
                    dest.y = m->value_y + 1;

                    SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);
                }

                if(d7->leftmost_position < int(d7->text.size()) - d7->visible_chars)
                {
                    src.x = 80;
                    src.y = 80;
                    if(m->value_text_flags & DRAWTEXT_ALIGN_RIGHT)
                    {
                        dest.x = m->value_x;
                    }
                    else
                    {
                        dest.x = m->value_x + ((m->value_text_flags & DRAWTEXT_THIN_FONT ? 13 : 16) * d7->visible_chars);
                    }
                    dest.y = m->value_y + 1;

                    SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);
                }

                fmt = text_fmt_create(m->value_text_flags, m->value_text_rgba, RGBA_OUTLINE_DEFAULT);
                monofont = monofont_square;

                if(m->value_text_flags & DRAWTEXT_THIN_FONT)
                    monofont = monofont_thin;
                if(m->value_text_flags & DRAWTEXT_SMALL_FONT)
                    monofont = monofont_small;
                if(m->value_text_flags & DRAWTEXT_TINY_FONT)
                    monofont = monofont_tiny;
                if(m->value_text_flags & DRAWTEXT_FIXEDSYS_FONT)
                    monofont = monofont_fixedsys;

                gfx_drawtext(cs, textinput_display, m->value_x, m->value_y + 1, monofont, &fmt);

                if(d7->active)
                {
                    if(m->value_text_flags & DRAWTEXT_THIN_FONT)
                    {
                        src.x = 15 * 13;
                        src.y = 2 * 18;
                        if(m->value_text_flags & DRAWTEXT_ALIGN_RIGHT)
                            dest.x = static_cast<int>(std::size_t(m->value_x) - 13 * ((int(d7->text.size()) > d7->visible_chars ? std::size_t(d7->visible_chars) : d7->text.size())) + 13 * ((long long)d7->position - d7->leftmost_position));
                        else
                            dest.x = m->value_x + 13 * (d7->position - d7->leftmost_position);
                        dest.y = m->value_y + 18;

                        src.w = 13;
                        dest.w = 13;
                        src.h = 18;
                        dest.h = 18;

                        SDL_RenderCopy(cs->screen.renderer, font_thin, &src, &dest);

                        src.w = 16;
                        dest.w = 16;
                        src.h = 16;
                        dest.h = 16;
                    }
                    else
                    {
                        src.x = 15 * 16;
                        src.y = 32;
                        if(m->value_text_flags & DRAWTEXT_ALIGN_RIGHT)
                            dest.x = static_cast<int>(m->value_x - 16 * ((int(d7->text.size()) > d7->visible_chars ? d7->visible_chars : d7->text.size())) + 16 * ((long long)d7->position - d7->leftmost_position));
                        else
                            dest.x = m->value_x + 16 * (d7->position - d7->leftmost_position);
                        dest.y = m->value_y + 16;

                        SDL_RenderCopy(cs->screen.renderer, font, &src, &dest);
                    }
                }
            }
        }

        if(m->type == Shiro::ElementType::MENU_TOGGLE)
        {
            d8 = (Shiro::ToggleOptionData *)m->data;

            fmt = text_fmt_create(m->value_text_flags, m->value_text_rgba, RGBA_OUTLINE_DEFAULT);
            monofont = monofont_square;

            if(m->value_text_flags & DRAWTEXT_THIN_FONT)
                monofont = monofont_thin;
            if(m->value_text_flags & DRAWTEXT_SMALL_FONT)
                monofont = monofont_small;
            if(m->value_text_flags & DRAWTEXT_TINY_FONT)
                monofont = monofont_tiny;
            if(m->value_text_flags & DRAWTEXT_FIXEDSYS_FONT)
                monofont = monofont_fixedsys;

            if(*(d8->param))
                gfx_drawtext(cs, d8->labels[1], m->value_x, m->value_y, monofont, &fmt);
            else
                gfx_drawtext(cs, d8->labels[0], m->value_x, m->value_y, monofont, &fmt);
        }
    }

    SDL_SetRenderTarget(cs->screen.renderer, cs->screen.target_tex);
    SDL_SetRenderDrawColor(cs->screen.renderer, 0, 0, 0, 255);

    if(d->use_target_tex)
    {
        for(i = 0; i < d->numopts; i++)
        {
            d->menu[i].render_update = 0;
        }

        SDL_RenderCopy(cs->screen.renderer, d->target_tex, NULL, NULL);
    }

    return 0;
}