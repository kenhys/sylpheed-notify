#include <glib.h>
#include <libnotify/notify.h>

int main(int argc, char *argv[])
{
  gboolean succeed;
  NotifyNotification *notify;

  succeed = notify_init("libnotify sample");
  if (!succeed) {
    return -1;
  }
  notify = notify_notification_new("Title",
                                   "This is an example notification.",
                                   "dialog-information");

  notify_notification_show(notify, NULL);
  g_object_unref(G_OBJECT(notify));

  notify_uninit();

  return 0;
}

