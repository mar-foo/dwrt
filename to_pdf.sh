#!/bin/sh -e
if test -z $1; then
	echo "usage: $0 variable" >&2
	exit 1
fi

read -r func

cat <<EOF | pdflatex >/dev/null
\documentclass{standalone}
\begin{document}
\$$(echo "$func" | ./derive -l "$1")\$
\end{document}
EOF

zathura texput.pdf ; rm -f texput.*
