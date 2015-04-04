#include <glib.h>
#include <libappindicator/app-indicator.h>
#include <gtk/gtk.h>

int main(int argc, char *argv[])
{
  gboolean succeed;
  GtkWidget *tray_menu;
  GtkWidget *menu_item;
  AppIndicator *indicator;

  gtk_init(&argc, &argv);

  indicator = app_indicator_new("sample indicator",
                                "orca",
                                APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);

  tray_menu = gtk_menu_new();
  menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
  gtk_menu_shell_append((GtkMenuShell*)tray_menu, menu_item);
  app_indicator_set_menu(indicator, GTK_MENU(tray_menu));
  gtk_widget_show_all(tray_menu);
  return 0;
}

