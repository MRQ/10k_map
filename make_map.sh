#!/bin/bash

sed -e "
	s/<!-- insert paths here -->//
	T noinsert
	r ${1}
	: noinsert
" < template.svg
