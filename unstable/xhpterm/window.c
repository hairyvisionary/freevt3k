#include <stdio.h>
#include <gtk/gtk.h>

#include "x11glue.h"
#include "conmgr.h"
#include "hpterm.h"
#include "../typedef.h"
/*#include "../vtconn.h"*/
#include "terminal.bm"

/* Globals */
  const gchar *F1_label = "F1";
  const gchar *F2_label = "F2"; 
void
on_window_destroy (GtkWidget *widget, gpointer data)
{
  gtk_main_quit ();
}

/* Callback for close button */
void
on_button_clicked (GtkWidget *F1_button, GtkTextBuffer *messageBuffer)
{
  GtkTextIter start;
  GtkTextIter end;

  gchar *text;

  /* Obtain iters for the start and end of points of the buffer */
  gtk_text_buffer_get_start_iter (messageBuffer, &start);
  gtk_text_buffer_get_end_iter (messageBuffer, &end);

  /* Get the entire buffer text. */
  text = gtk_text_buffer_get_text (messageBuffer, &start, &end, FALSE);

  /* Print the text */
  /* g_print ("%s", text);

  g_free (text); */

  F1_label = "clicked";
  F2_label = "changed"; 
  
  void gtk_button_set_label (GtkButton *F1_button,
			   const gchar *F1_label);
  void gtk_button_set_label (GtkButton *F2_button,
			   const gchar *F2_label);

  
}

int 
main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
  GtkWidget *hbox;
  
  GtkWidget *group1;
  GtkWidget *group2;
  
  GtkWidget *text_view;
  
  GtkWidget *F1_button;
  GtkWidget *F2_button;
  GtkWidget *F3_button;
  GtkWidget *F4_button;
  GtkWidget *F5_button;
  GtkWidget *F6_button;
  GtkWidget *F7_button;
  GtkWidget *F8_button;
  GtkTextBuffer *messageBuffer;		/* using the buffer from freevt3k.c and vt3lglue.c */

  /* test case for changing the function button labels */
    
  gtk_init (&argc, &argv);

  /* Create a Window. */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "xHPTerm");

  /* Set a decent default size for the window. */
  gtk_window_set_default_size (GTK_WINDOW (window), 640, 300);
  g_signal_connect (G_OBJECT (window), "destroy", 
                    G_CALLBACK (on_window_destroy),
                    NULL);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  
  /* making the window scrollable */
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

  /* Create a multiline text widget. */
  text_view = gtk_text_view_new ();
  gtk_scrolled_window_add_with_viewport (
                   GTK_SCROLLED_WINDOW (scrolled_window), text_view);

  /* Obtaining the buffer associated with the widget. */
  messageBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
  /* Set the default buffer text. */ 
  gtk_text_buffer_set_text (messageBuffer, "Hello Text View!", -1);
  
  /* Create a close button. */
  hbox = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, 0, 0, 0);

  group1 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX (hbox), group1, 1, 1, 5);
  
  group2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX (hbox), group2, 1, 1, 5);

  F1_button = gtk_button_new_with_label (F1_label);
  gtk_box_pack_start (GTK_BOX (group1), F1_button, 1, 1, 0);
  g_signal_connect (G_OBJECT (F1_button), "clicked", 
                    G_CALLBACK (on_button_clicked),
                    messageBuffer);
  
  F2_button = gtk_button_new_with_label (F2_label);
  gtk_box_pack_start (GTK_BOX (group1), F2_button, 1, 1, 0);
  
  F3_button = gtk_button_new_with_label ("F3");
  gtk_box_pack_start (GTK_BOX (group1), F3_button, 1, 1, 0);
  
  F4_button = gtk_button_new_with_label ("F4");
  gtk_box_pack_start (GTK_BOX (group1), F4_button, 1, 1, 0);
  
  F5_button = gtk_button_new_with_label ("F5");
  gtk_box_pack_start (GTK_BOX (group2), F5_button, 1, 1, 0);
  
  F6_button = gtk_button_new_with_label ("F6");
  gtk_box_pack_start (GTK_BOX (group2), F6_button, 1, 1, 0);
  
  F7_button = gtk_button_new_with_label ("F7");
  gtk_box_pack_start (GTK_BOX (group2), F7_button, 1, 1, 0);
  
  F8_button = gtk_button_new_with_label ("F8");
  gtk_box_pack_start (GTK_BOX (group2), F8_button, 1, 1, 0);
  
  gtk_widget_show_all (window);

  gtk_main ();
  return 0;
}
