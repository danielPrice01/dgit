Simple clone of git.

Written in pure C, generally following a [tutorial by Nikita](https://www.leshenko.net/p/ugit/#)

Does not rely on any external libraries.

Run `make` and use any of the [supported commands](#supported-commands):

---

### Supported Commands:

`
./dgit init
`
- Creates .dgit directory containing:
  - .objects

`
./dgit hash-object <filename>
`
- Uses SHA1 algorithm to hash file contents
- Creates file with generated hash file contents
- Writes contents of <filename> to newly created file

`
./dgit cat-file <filename>
`
- Searches for \<filename> (SHA1 hash) in .dgit/objects
- Prints contents of file to terminal

`
./dgit write-tree
`
- Recursively adds directory tree objects to .dgit/objects
