/*
 *  Copyright (C) 2015-2017 Gooroom <gooroom@gooroom.kr>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef __GRMKIT_BUTTON_H__
#define __GRMKIT_BUTTON_H__

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GRMKIT_TYPE_BUTTON            (grmkit_button_get_type ())
#define GRMKIT_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRMKIT_TYPE_BUTTON, GrmkitButton))
#define GRMKIT_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRMKIT_TYPE_BUTTON, GrmkitButtonClass))
#define GRMKIT_IS_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRMKIT_TYPE_BUTTON))
#define GRMKIT_IS_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRMKIT_TYPE_BUTTON))
#define GRMKIT_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRMKIT_TYPE_BUTTON, GrmkitButtonClass))

typedef struct _GrmkitButtonPrivate GrmkitButtonPrivate;
typedef struct _GrmkitButtonClass   GrmkitButtonClass;
typedef struct _GrmkitButton        GrmkitButton;

struct _GrmkitButtonClass
{
    GtkButtonClass __parent__;
};

struct _GrmkitButton
{
    GtkButton __parent__;
    GrmkitButtonPrivate *priv;
};

GType         grmkit_button_get_type   (void) G_GNUC_CONST;

GrmkitButton *grmkit_button_new        (void);

void    grmkit_button_set_title (GrmkitButton *button, const char* szTitle);
void    grmkit_button_set_description (GrmkitButton *button, const char* szDesc);
void    grmkit_button_set_icon_from_file (GrmkitButton *button, const char* szFilePath);
void	grmkit_button_set_index (GrmkitButton *button, int index);
int	grmkit_button_get_index (GrmkitButton *button);

G_END_DECLS

#endif /* !__GRMKIT_BUTTON_H__ */
