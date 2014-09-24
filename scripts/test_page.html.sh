#!/bin/bash
page="$1"
output="$2"

cat >/tmp/$( basename "$0").tmp.sh <<EOF
#!/bin/bash
cat <<EOFF
<html>
<head>
<!-- Latest compiled and minified CSS -->
<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/css/bootstrap.min.css">

<!-- Optional theme -->
<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/css/bootstrap-theme.min.css">

<!-- Latest compiled and minified JavaScript -->
<script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/js/bootstrap.min.js"></script>

<link rel="stylesheet" href="$( readlink -f "resources/style.css" )">
$header_text
</head>
<body>
$body_pre
$( cat "$page")
$body_after
</body>
</html>
EOFF
EOF

chmod a+x /tmp/$( basename "$0").tmp.sh
. /tmp/$( basename "$0").tmp.sh > "$output"

