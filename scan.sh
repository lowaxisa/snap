#!/bin/bash
SOURCE=""  # files source
IGNORE=("dump.txt" ".temp" ".vscode" ".git") # ignore files array

is_ignore() {
	local file="$1"
	local item=""
    local filename
    filename=$(basename "$file")

	for item in "${IGNORE[@]}"; do
		if [[ "$filename" == "$item" ]]; then
			return 0
		fi
	done

	return 1
}

# reading tree
SOURCE=$'--- FILE TREE ---\n'
SOURCE="${SOURCE}$(tree .)"$'\n'

# reading files
SOURCE="${SOURCE}"$'\n'"--- FILES CONTENT ---"$'\n'

scan() {
	local path="$1"
	local item=""

	for item in "${path}"/*; do
		if [[ -f "$item" ]]; then # -f is file
			if is_ignore "$item"; then
				continue
			fi
			SOURCE="${SOURCE}FILE: $item"
			SOURCE="${SOURCE}"$'\n'
			SOURCE="${SOURCE}$(cat $item)"$'\n'
		elif [[ -d "$item" ]]; then
			scan "${item}"
		fi
	done
}

scan "."
echo "$SOURCE" > dump.txt