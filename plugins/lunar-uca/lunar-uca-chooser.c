/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include <gdk/gdkkeysyms.h>

#include <lunar-uca/lunar-uca-chooser.h>
#include <lunar-uca/lunar-uca-editor.h>
#include <lunar-uca/lunar-uca-model.h>



static gboolean lunar_uca_chooser_key_press_event    (GtkWidget              *widget,
                                                       GdkEventKey            *event);
static void     lunar_uca_chooser_response           (GtkDialog              *dialog,
                                                       gint                    response);
static void     lunar_uca_chooser_exchange           (LunarUcaChooser       *uca_chooser,
                                                       GtkTreeSelection       *selection,
                                                       GtkTreeModel           *model,
                                                       GtkTreeIter            *iter_a,
                                                       GtkTreeIter            *iter_b);
static void     lunar_uca_chooser_open_editor        (LunarUcaChooser       *uca_chooser,
                                                       gboolean                edit);
static void     lunar_uca_chooser_save               (LunarUcaChooser       *uca_chooser,
                                                       LunarUcaModel         *uca_model);
static void     lunar_uca_chooser_selection_changed  (LunarUcaChooser       *uca_chooser,
                                                       GtkTreeSelection       *selection);
static void     lunar_uca_chooser_add_clicked        (LunarUcaChooser       *uca_chooser);
static void     lunar_uca_chooser_edit_clicked       (LunarUcaChooser       *uca_chooser);
static void     lunar_uca_chooser_delete_clicked     (LunarUcaChooser       *uca_chooser);
static void     lunar_uca_chooser_up_clicked         (LunarUcaChooser       *uca_chooser);
static void     lunar_uca_chooser_down_clicked       (LunarUcaChooser       *uca_chooser);



struct _LunarUcaChooserClass
{
  GtkDialogClass __parent__;
};

struct _LunarUcaChooser
{
  GtkDialog __parent__;

  GtkWidget   *treeview;
  GtkWidget   *add_button;
  GtkWidget   *edit_button;
  GtkWidget   *delete_button;
  GtkWidget   *up_button;
  GtkWidget   *down_button;
};



LUNARX_DEFINE_TYPE (LunarUcaChooser, lunar_uca_chooser, GTK_TYPE_DIALOG);



static void
lunar_uca_chooser_class_init (LunarUcaChooserClass *klass)
{
  GtkDialogClass *dialog_class;
  GtkWidgetClass *widget_class;

  widget_class = GTK_WIDGET_CLASS (klass);
  widget_class->key_press_event = lunar_uca_chooser_key_press_event;

  dialog_class = GTK_DIALOG_CLASS (klass);
  dialog_class->response = lunar_uca_chooser_response;

  /* Setup the template xml */
  gtk_widget_class_set_template_from_resource (widget_class, "/com/expidus/lunar/uca/chooser.ui");

  /* bind stuff */
  gtk_widget_class_bind_template_child (widget_class, LunarUcaChooser, treeview);
  gtk_widget_class_bind_template_child (widget_class, LunarUcaChooser, add_button);
  gtk_widget_class_bind_template_child (widget_class, LunarUcaChooser, edit_button);
  gtk_widget_class_bind_template_child (widget_class, LunarUcaChooser, delete_button);
  gtk_widget_class_bind_template_child (widget_class, LunarUcaChooser, up_button);
  gtk_widget_class_bind_template_child (widget_class, LunarUcaChooser, down_button);

  gtk_widget_class_bind_template_callback(widget_class, lunar_uca_chooser_add_clicked);
  gtk_widget_class_bind_template_callback(widget_class, lunar_uca_chooser_edit_clicked);
  gtk_widget_class_bind_template_callback(widget_class, lunar_uca_chooser_delete_clicked);
  gtk_widget_class_bind_template_callback(widget_class, lunar_uca_chooser_up_clicked);
  gtk_widget_class_bind_template_callback(widget_class, lunar_uca_chooser_down_clicked);
  gtk_widget_class_bind_template_callback(widget_class, lunar_uca_chooser_selection_changed);
}



static void
lunar_uca_chooser_init (LunarUcaChooser *uca_chooser)
{
  GtkTreeViewColumn *column;
  GtkCellRenderer   *renderer;
  LunarUcaModel    *uca_model;
  gboolean           have_header_bar;

  /* Initialize the template for this instance */
  gtk_widget_init_template (GTK_WIDGET (uca_chooser));

  /* configure the dialog window */
  g_object_get (uca_chooser, "use-header-bar", &have_header_bar, NULL);

  if (!have_header_bar)
    {
      /* add a regular close button, the header bar already provides one */
      gtk_dialog_add_button (GTK_DIALOG (uca_chooser), _("_Close"), GTK_RESPONSE_CLOSE);
    }

  gtk_dialog_set_default_response (GTK_DIALOG (uca_chooser), GTK_RESPONSE_CLOSE);

  /* configure the tree view */
  uca_model = lunar_uca_model_get_default ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (uca_chooser->treeview), GTK_TREE_MODEL (uca_model));
  g_object_unref (G_OBJECT (uca_model));

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_expand (column, TRUE);
  gtk_tree_view_column_set_resizable (column, FALSE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (uca_chooser->treeview), column);

  renderer = g_object_new (GTK_TYPE_CELL_RENDERER_PIXBUF, "stock-size", GTK_ICON_SIZE_DND, "xpad", 2, "ypad", 2, NULL);
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer, "gicon", LUNAR_UCA_MODEL_COLUMN_GICON, NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_set_attributes (column, renderer, "markup", LUNAR_UCA_MODEL_COLUMN_STOCK_LABEL, NULL);

  /* configure the tree view selection */
  lunar_uca_chooser_selection_changed (uca_chooser, gtk_tree_view_get_selection (GTK_TREE_VIEW (uca_chooser->treeview)));
}



static gboolean
lunar_uca_chooser_key_press_event (GtkWidget   *widget,
                                    GdkEventKey *event)
{
  /* close chooser window on Esc key press */
  if (G_UNLIKELY (event->keyval == GDK_KEY_Escape))
    {
      gtk_dialog_response (GTK_DIALOG (widget), GTK_RESPONSE_CLOSE);
      return TRUE;
    }

  return (*GTK_WIDGET_CLASS (lunar_uca_chooser_parent_class)->key_press_event) (widget, event);
}



static void
lunar_uca_chooser_response (GtkDialog *dialog,
                             gint       response)
{
  if (response == GTK_RESPONSE_CLOSE)
    {
      gtk_widget_destroy (GTK_WIDGET (dialog));
    }
  else if (GTK_DIALOG_CLASS (lunar_uca_chooser_parent_class)->response != NULL)
    {
      (*GTK_DIALOG_CLASS (lunar_uca_chooser_parent_class)->response) (dialog, response);
    }
}



static void
lunar_uca_chooser_selection_changed (LunarUcaChooser *uca_chooser,
                                      GtkTreeSelection *selection)
{
  GtkTreeModel *model;
  GtkTreePath  *path = NULL;
  GtkTreeIter   iter;
  gboolean      selected;

  g_return_if_fail (LUNAR_UCA_IS_CHOOSER (uca_chooser));
  g_return_if_fail (GTK_IS_TREE_SELECTION (selection));

  /* check if we have currently selected an item */
  selected = gtk_tree_selection_get_selected (selection, &model, &iter);
  if (G_LIKELY (selected))
    {
      /* determine the path for the selected iter */
      path = gtk_tree_model_get_path (model, &iter);
    }

  /* change sensitivity of "Edit" and "Delete" appropritately */
  gtk_widget_set_sensitive (uca_chooser->edit_button, selected);
  gtk_widget_set_sensitive (uca_chooser->delete_button, selected);

  /* change sensitivity of "Move Up" and "Move Down" appropritately */
  gtk_widget_set_sensitive (uca_chooser->up_button, selected && gtk_tree_path_get_indices (path)[0] > 0);
  gtk_widget_set_sensitive (uca_chooser->down_button, selected && gtk_tree_path_get_indices (path)[0] < gtk_tree_model_iter_n_children (model, NULL) - 1);

  /* release path (if any) */
  if (G_LIKELY (path != NULL))
    gtk_tree_path_free (path);
}



static void
lunar_uca_chooser_exchange (LunarUcaChooser *uca_chooser,
                             GtkTreeSelection *selection,
                             GtkTreeModel     *model,
                             GtkTreeIter      *iter_a,
                             GtkTreeIter      *iter_b)
{
  g_return_if_fail (LUNAR_UCA_IS_CHOOSER (uca_chooser));
  g_return_if_fail (GTK_IS_TREE_SELECTION (selection));
  g_return_if_fail (GTK_IS_TREE_MODEL (model));
  g_return_if_fail (iter_a != NULL);
  g_return_if_fail (iter_b != NULL);

  /* perform the move */
  lunar_uca_model_exchange (LUNAR_UCA_MODEL (model), iter_a, iter_b);

  /* tell the chooser that the selection may have changed */
  lunar_uca_chooser_selection_changed (uca_chooser, selection);

  /* sync the model to persistent storage */
  lunar_uca_chooser_save (uca_chooser, LUNAR_UCA_MODEL (model));
}



static void
lunar_uca_chooser_open_editor (LunarUcaChooser *uca_chooser,
                                gboolean          edit)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  GtkWidget        *editor;
  gboolean          use_header_bar = FALSE;

  g_return_if_fail (LUNAR_UCA_IS_CHOOSER (uca_chooser));

  /* allocate the new editor */
  g_object_get (gtk_settings_get_for_screen (gtk_widget_get_screen (GTK_WIDGET (uca_chooser))),
                "gtk-dialogs-use-header", &use_header_bar, NULL);

  editor = g_object_new (LUNAR_UCA_TYPE_EDITOR, "use-header-bar", use_header_bar, NULL);
  gtk_window_set_title (GTK_WINDOW (editor), edit ? _("Edit Action") : _("Create Action"));
  gtk_window_set_transient_for (GTK_WINDOW (editor), GTK_WINDOW (uca_chooser));

  /* load the editor with the currently selected item (when editing) */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (uca_chooser->treeview));
  if (gtk_tree_selection_get_selected (selection, &model, &iter) && edit)
    lunar_uca_editor_load (LUNAR_UCA_EDITOR (editor), LUNAR_UCA_MODEL (model), &iter);

  /* run the editor */
  if (gtk_dialog_run (GTK_DIALOG (editor)) == GTK_RESPONSE_OK)
    {
      /* append a new iter (when not editing) */
      if (G_UNLIKELY (!edit))
        lunar_uca_model_append (LUNAR_UCA_MODEL (model), &iter);

      /* save the editor values to the model */
      lunar_uca_editor_save (LUNAR_UCA_EDITOR (editor), LUNAR_UCA_MODEL (model), &iter);

      /* hide the editor window */
      gtk_widget_hide (editor);

      /* sync the model to persistent storage */
      lunar_uca_chooser_save (uca_chooser, LUNAR_UCA_MODEL (model));
    }

  /* destroy the editor */
  gtk_widget_destroy (editor);
}



static void
lunar_uca_chooser_save (LunarUcaChooser *uca_chooser,
                         LunarUcaModel   *uca_model)
{
  GtkWidget *dialog;
  GError    *error = NULL;

  g_return_if_fail (LUNAR_UCA_IS_CHOOSER (uca_chooser));
  g_return_if_fail (LUNAR_UCA_IS_MODEL (uca_model));

  /* sync the model to persistent storage */
  if (!lunar_uca_model_save (uca_model, &error))
    {
      dialog = gtk_message_dialog_new (GTK_WINDOW (uca_chooser),
                                       GTK_DIALOG_DESTROY_WITH_PARENT
                                       | GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_CLOSE,
                                       _("Failed to save actions to disk."));
      gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s.", error->message);
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      g_error_free (error);
    }
}



static void
lunar_uca_chooser_add_clicked (LunarUcaChooser *uca_chooser)
{
  g_return_if_fail (LUNAR_UCA_IS_CHOOSER (uca_chooser));
  lunar_uca_chooser_open_editor (uca_chooser, FALSE);
}



static void
lunar_uca_chooser_edit_clicked (LunarUcaChooser *uca_chooser)
{
  g_return_if_fail (LUNAR_UCA_IS_CHOOSER (uca_chooser));
  lunar_uca_chooser_open_editor (uca_chooser, TRUE);
}



static void
lunar_uca_chooser_delete_clicked (LunarUcaChooser *uca_chooser)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  gchar            *name;
  GtkWidget        *dialog;
  gint              response;

  g_return_if_fail (LUNAR_UCA_IS_CHOOSER (uca_chooser));

  /* verify that we have an item selected and determine the iter for that item */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (uca_chooser->treeview));
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      /* create the question dialog */
      gtk_tree_model_get (model, &iter, LUNAR_UCA_MODEL_COLUMN_NAME, &name, -1);
      dialog = gtk_message_dialog_new (GTK_WINDOW (uca_chooser),
                                       GTK_DIALOG_DESTROY_WITH_PARENT
                                       | GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_QUESTION,
                                       GTK_BUTTONS_NONE,
                                       _("Are you sure that you want to delete\naction \"%s\"?"), name);
      gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), _("If you delete a custom action, it is permanently lost."));
      gtk_dialog_add_buttons (GTK_DIALOG (dialog), _("_Cancel"), GTK_RESPONSE_CANCEL,
                              _("_Delete"), GTK_RESPONSE_YES, NULL);
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
      g_free (name);
      response = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);

      if (response == GTK_RESPONSE_YES)
        {
          /* remove the row from the model */
          lunar_uca_model_remove (LUNAR_UCA_MODEL (model), &iter);

          /* sync the model to persistent storage */
          lunar_uca_chooser_save (uca_chooser, LUNAR_UCA_MODEL (model));
        }
    }
}



static void
lunar_uca_chooser_up_clicked (LunarUcaChooser *uca_chooser)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreePath      *path;
  GtkTreeIter       iter_a;
  GtkTreeIter       iter_b;

  g_return_if_fail (LUNAR_UCA_IS_CHOOSER (uca_chooser));

  /* determine the currently selected item */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (uca_chooser->treeview));
  if (gtk_tree_selection_get_selected (selection, &model, &iter_a))
    {
      /* determine the tree path to the iter */
      path = gtk_tree_model_get_path (model, &iter_a);
      if (gtk_tree_path_prev (path))
        {
          /* determine the iter for the previous item */
          gtk_tree_model_get_iter (model, &iter_b, path);

          /* perform the exchange operation */
          lunar_uca_chooser_exchange (uca_chooser, selection, model, &iter_a, &iter_b);
        }

      /* release the path */
      gtk_tree_path_free (path);
    }
}



static void
lunar_uca_chooser_down_clicked (LunarUcaChooser *uca_chooser)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter_a;
  GtkTreeIter       iter_b;

  g_return_if_fail (LUNAR_UCA_IS_CHOOSER (uca_chooser));

  /* determine the currently selected item */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (uca_chooser->treeview));
  if (gtk_tree_selection_get_selected (selection, &model, &iter_a))
    {
      /* determine the iter to the next item */
      iter_b = iter_a;
      if (gtk_tree_model_iter_next (model, &iter_b))
        lunar_uca_chooser_exchange (uca_chooser, selection, model, &iter_a, &iter_b);
    }
}




