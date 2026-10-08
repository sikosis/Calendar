![Calendar Icon](documentation/images/calendar_icon_64.png) **Calendar** for [Haiku](https://www.haiku-os.org/).

* * *

Calendar is a native Haiku application to manage your appointments.    
You can create, edit and delete events, and cancel or hide them, categorize events and show them in a day, week or month view.

![screenshot](documentation/images/main_window.png)

Note
-------
When building on 32bit Haiku, you need to change the build environment to use gcc11 with `setarch x86` before invoking `make`.

Build
-------

From the `main` directory, run:

```
make
make bindcatalogs
```

Tests
-------

The platform-independent tests can also be run outside Haiku:

```
cd tests
make test
```


For more information, please see the [Calendar documentation](http://htmlpreview.github.io/?https://github.com/HaikuArchives/Calendar/master/documentation/Documentation.html).
