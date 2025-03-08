$content = Get-Content "ggxrd_freecam_dll.rc"
$newline = "`
"
$tab = "	";

. "..\common\prebuild.ps1"

$VERSION = parse_version("Version.h")

$content_lines = $content -split $newline
$need_write = $false

$version_parts = $VERSION.Split(".");
$new_version = $version_parts[0] + ",0," + $version_parts[1] + "," + $version_parts[2];

if (replace_file_and_product_versions $content_lines $new_version) {
	$need_write = $true;
}

if ($need_write) {
	Set-Content -Value ($content_lines -join $newline) "ggxrd_freecam_dll.rc" -Encoding Unicode
}
