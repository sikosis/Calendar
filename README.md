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

Package
-------

On Haiku, build and test an installable package from the repository root:

```
packaging/build-hpkg.sh
```

The package is written to `packages/calendar-0.2.0-3-<architecture>.hpkg`.
Install it with `pkgman`, substituting the generated filename:

```
pkgman install ./packages/calendar-0.2.0-3-x86_64.hpkg
```

For a secondary 32-bit build on 64-bit Haiku, run:

```
setarch x86 packaging/build-hpkg.sh
```

The builder does not overwrite an existing package of the same version and
revision. Move the existing package aside or increase the package revision
before rebuilding it.


For more information, please see the [Calendar documentation](documentation/Documentation.html).
