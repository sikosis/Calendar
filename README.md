![Calendar Icon](documentation/images/calendar_icon_64.png) **Calendar** for [Haiku](https://www.haiku-os.org/).

* * *

Calendar is a native Haiku application to manage your appointments.

You can create, edit and delete events, and cancel or hide them, categorize
events and show them in a day, week or month view. Calendar also now includes a
small Deskbar calendar replicant icon with the current date. As well as other immprovements,
the Event menu provides Calendar's own scheduled task launcher.

![screenshot](documentation/images/main_window.png)

This project was originally written by a dedicated team of talented coders who laid a strong and thoughtful foundation. Over time, however, the code fell dormant — left unmaintained, its dependencies stale and its tooling obsolete. Rather than start over, I have brought new life to the project: modernizing the existing base, reviving its momentum, and extending the application toward its original promise. The goal is simple: to make it the best app on Haiku. Credit to the original authors, whose work this builds directly upon.

---

**Sikosis**
Maintainer · [GitHub](https://github.com/sikosis)

| **Status** | Active development |

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

Package
-------

On Haiku, build and test an installable package from the repository root:

```
packaging/build-hpkg.sh
```

The package is written to `packages/calendar-0.6.0-1-<architecture>.hpkg`.
Install it with `pkgman`, substituting the generated filename:

```
pkgman install ./packages/calendar-0.6.0-1-x86_64.hpkg
```

Calendar stores and launches scheduled tasks itself. Its persistent Deskbar
replicant checks tasks once per minute, including while the main window is
closed.

For a secondary 32-bit build on 64-bit Haiku, run:

```
setarch x86 packaging/build-hpkg.sh
```

The builder does not overwrite an existing package of the same version and
revision. Move the existing package aside or increase the package revision
before rebuilding it.

Scripting
---------

Calendar supports Haiku scripting through `hey`. Use `hey Calendar GETSUITES`
to inspect the complete vocabulary. Examples:

```
hey Calendar GET Version
hey Calendar GET SelectedDate
hey Calendar SET SelectedDate TO "2026-10-15"
hey Calendar GET View
hey Calendar SET View TO week
hey Calendar DO Today
hey Calendar DO NewEvent
hey Calendar DO ManageCategories
```


For more information, please see the [Calendar documentation](documentation/Documentation.html).
