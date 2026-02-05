Simple clone of git.
Run `make` and use any of following commands:

---

### Supports:

`
./dgit init
`
- Creates .dgit directory containing:
  - .objects

`
./dgit hash-object <filename>
`
- Uses SHA1 algorithm to hash filename
- Creates file with generated hash filename
- Writes contents of <filename> to newly created file

`
./dgit cat-file <filename>
`
- Searches for \<filename> (SHA1 hash) in .dgit/objects
- Prints contents of file to terminal

`
./dgit write-tree
`
- Recursively adds all files and subdirectories to .dgit/objects
