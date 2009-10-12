#include <gtk/gtk.h>

void
on_window_destroy (GtkWidget *widget, gpointer data)
{
  gtk_main_quit ();
}

/* Callback for buttons in the toolbar. */
void
on_format_button_clicked (GtkWidget *button, GtkTextBuffer *buffer)
{
  GtkTextIter start, end;
  gchar *tag_name;

  /* Get iters at the beginning and end of current selection. */
  gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  /* Find out what tag to apply. The key "tag" is set to the
     appropriate tag name when the button widget was created. */
  tag_name = g_object_get_data (G_OBJECT (button), "tag");
  /* Apply the tag to the selected text. */
  gtk_text_buffer_apply_tag_by_name (buffer, tag_name, &start, &end);
}

/* Callback for the close button. */
void
on_close_button_clicked (GtkWidget *button, GtkTextBuffer *buffer)
{
  GtkTextIter start;
  GtkTextIter end;

  gchar *text;

  /* Get iters at the beginning and end of the buffer. */
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  /* Retrieve the text. */
  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  /* Print the text. */
  g_print ("%s", text);

  g_free (text);

  gtk_main_quit ();
}

int 
main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *bbox;
  GtkWidget *scrolled_window;

  GtkWidget *bold_button;
  GtkWidget *italic_button;
  GtkWidget *font_button;

  GtkWidget *text_view;
  GtkTextBuffer *buffer;

  GtkWidget *close_button;
  
  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Formatted multiline text widget");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  g_signal_connect (G_OBJECT (window), "destroy", 
                    G_CALLBACK (on_window_destroy),
                    NULL);

  vbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  /* Create a button box that will serve as a toolbar. */
  bbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX (vbox), bbox, 0, 0, 0);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    
  gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 0);
    
    /* the policy is one of GTK_POLICY AUTOMATIC, or GTK_POLICY_ALWAYS.
     * GTK_POLICY_AUTOMATIC will automatically decide whether you need
     * scrollbars, whereas GTK_POLICY_ALWAYS will always leave the scrollbars
     * there.  The first one is the horizontal scrollbar, the second, 
     * the vertical. */
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
   /* The dialog window is created with a vbox packed into it. */								
  gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, 
			TRUE, TRUE, 0);

  /* Create the text view widget and set some default text. */
  text_view = gtk_text_view_new ();
  gtk_scrolled_window_add_with_viewport (
                   GTK_SCROLLED_WINDOW (scrolled_window), text_view);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
  gtk_text_buffer_set_text (buffer, "Hello Text View!", -1);

  /* Create tags associated with the buffer. */
  /* Tag with weight bold and tag name "bold" . */
  gtk_text_buffer_create_tag (buffer, "bold", 
                              "weight", PANGO_WEIGHT_BOLD, 
                              NULL);
  /* Tag with style italic and tag name "italic". */
  gtk_text_buffer_create_tag (buffer, "italic",
                              "style", PANGO_STYLE_ITALIC,
                              NULL);
  /* Tag with font fixed and tag name "font". */
  gtk_text_buffer_create_tag (buffer, "font",
                              "font", "fixed", 
                              NULL); 

  /* Create button for bold and add them the to the tool bar. */
  bold_button = gtk_button_new_with_label ("Bold");
  gtk_container_add (GTK_CONTAINER (bbox), bold_button);
  /* Connect the common signal handler on_format_button_clicked. This
     signal handler is common to all the buttons. A key called "tag"
     is associated with the buttons, which speicifies the tag name
     that the button is supposed to apply. The handler reads this key
     and applies the appropriate tag. Thus only one handler is needed
     for any number of buttons in the toolbar. */
  g_signal_connect (G_OBJECT (bold_button), "clicked",
                    G_CALLBACK (on_format_button_clicked),
                    buffer);
  g_object_set_data (G_OBJECT (bold_button), "tag", "bold");

  /* Create button for italic. */
  italic_button = gtk_button_new_with_label ("Italic");
  gtk_container_add (GTK_CONTAINER (bbox), italic_button);
  g_signal_connect (G_OBJECT (italic_button), "clicked",
                    G_CALLBACK (on_format_button_clicked),
                    buffer);
  g_object_set_data (G_OBJECT (italic_button), "tag", "italic");

  /* Create button for fixed font. */
  font_button = gtk_button_new_with_label ("Font Fixed");
  gtk_container_add (GTK_CONTAINER (bbox), font_button);
  g_signal_connect (G_OBJECT (font_button), "clicked",
                    G_CALLBACK (on_format_button_clicked),
                    buffer);
  g_object_set_data (G_OBJECT (font_button), "tag", "font");
  
  /* Create the close button. */
  close_button = gtk_button_new_with_label ("Close");
  gtk_box_pack_start (GTK_BOX (vbox), close_button, 0, 0, 0);
  g_signal_connect (G_OBJECT (close_button), "clicked", 
                    G_CALLBACK (on_close_button_clicked),
                    buffer);

  gtk_widget_show_all (window);

  gtk_main ();
  return 0;
}
