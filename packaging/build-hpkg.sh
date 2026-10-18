#!/bin/sh

set -eu

version="0.7.0"
revision="1"
scriptDirectory="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
sourceDirectory="$(CDPATH= cd -- "$scriptDirectory/.." && pwd)"
outputDirectory="$sourceDirectory/packages"
objectDirectory="objects-package"

for commandName in getarch make package sed; do
	if ! command -v "$commandName" >/dev/null 2>&1; then
		echo "Required command not found: $commandName" >&2
		exit 1
	fi
done

architecture="$(getarch)"
case "$architecture" in
	x86_64)
		haikuRequirement="haiku"
		;;
	x86)
		haikuRequirement="haiku_x86"
		;;
	x86_gcc2)
		echo "The GCC 2 build is not supported. Run this with setarch x86." >&2
		exit 1
		;;
	*)
		echo "Unsupported Haiku architecture: $architecture" >&2
		exit 1
		;;
esac

packageName="calendar_sikosis-$version-$revision-$architecture.hpkg"
outputFile="$outputDirectory/$packageName"
if [ -e "$outputFile" ]; then
	echo "Refusing to overwrite existing package: $outputFile" >&2
	exit 1
fi

echo "Running portable tests..."
make -C "$sourceDirectory/tests" clean test

echo "Building Calendar and binding translations..."
make -C "$sourceDirectory/main" OBJ_DIR="$objectDirectory" clean
make -C "$sourceDirectory/main" OBJ_DIR="$objectDirectory"
make -C "$sourceDirectory/main" OBJ_DIR="$objectDirectory" bindcatalogs

stageDirectory="$(mktemp -d "${TMPDIR:-/tmp}/calendar-package.XXXXXXXX")"
trap 'rm -rf "$stageDirectory"' EXIT HUP INT TERM

mkdir -p "$stageDirectory/apps"
mkdir -p "$stageDirectory/data/deskbar/menu/Applications"
mkdir -p "$stageDirectory/documentation/packages/calendar_sikosis"

cp "$sourceDirectory/main/$objectDirectory/Calendar" \
	"$stageDirectory/apps/Calendar"
cp -R "$sourceDirectory/documentation/." \
	"$stageDirectory/documentation/packages/calendar_sikosis"
cp "$sourceDirectory/LICENSE" \
	"$stageDirectory/documentation/packages/calendar_sikosis/LICENSE"
cp "$sourceDirectory/README.md" \
	"$stageDirectory/documentation/packages/calendar_sikosis/README.md"
ln -s ../../../../apps/Calendar \
	"$stageDirectory/data/deskbar/menu/Applications/Calendar"

sed \
	-e "s|@VERSION@|$version|g" \
	-e "s|@REVISION@|$revision|g" \
	-e "s|@ARCHITECTURE@|$architecture|g" \
	-e "s|@HAIKU_REQUIREMENT@|$haikuRequirement|g" \
	"$scriptDirectory/PackageInfo.in" > "$stageDirectory/.PackageInfo"

mkdir -p "$outputDirectory"
package create -C "$stageDirectory" "$outputFile"

echo "Created $outputFile"
