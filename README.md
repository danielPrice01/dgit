Simple clone of git.

---

### Supports:

`
./dgit init
`
- Creates .dgit directory with .objects

`
./dgit hash-object <filename>
`
- Uses SHA1 algorithm to hash filename
- Creates file with created hash
- Writes contents of <filename> to newly created file

`
./dgit cat-file <filename>
`
- Searches for <filename> (SHA1 hash) in .dgit/objects
- Prints contents of file to terminal

`
./dgit write-tree
`
- Recursively adds all files and subdirectories to .dgit/objects
