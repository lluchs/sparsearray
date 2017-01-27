#!/usr/bin/awk -f

/^data size/ { base = $4 }
/^start/ { name = $2 }
/^static size/ {
	# ChunkSA does not produce useful results due to dynamic allocation.
	if (name != "ChunkSA")
		print name, $4 / base
}
