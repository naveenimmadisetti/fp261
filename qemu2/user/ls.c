#include <inc/lib.h>

int flag[256];

void lsdir(const char*, const char*);
void ls1(const char*, bool, off_t, const char*);

void
ls(const char *path, const char *prefix)
{
	int r;
	struct Stat st;

	if ((r = stat(path, &st)) < 0)
		panic("stat %s: %e", path, r);
	if (st.st_ftype == FTYPE_DIR && !flag[(int) 'd'])
		lsdir(path, prefix);
	else
		ls1(0, st.st_ftype == FTYPE_DIR, st.st_size, path);
}

void
lsdir(const char *path, const char *prefix)
{
	int fd, n;
	char name[MAXNAMELEN];
	struct Direntry de;
	struct Stat st;

	if ((fd = open(path, O_RDONLY)) < 0)
		panic("open %s: %e", path, fd);
	while ((n = readn(fd, &de, sizeof de)) == sizeof de)
		if (de.de_inum) {
			memcpy(name, de.de_name, MAXNAMELEN);
			name[de.de_namelen] = 0;
			istat(de.de_inum, &st);
			ls1(prefix, st.st_ftype==FTYPE_DIR, st.st_size, name);
		}
	if (n > 0)
		panic("short read in directory %s", path);
	if (n < 0)
		panic("error reading directory %s: %e", path, n);
}

void
ls1(const char *prefix, bool isdir, off_t size, const char *name)
{
	const char *sep;

	if (flag[(int) 'l'])
		fprintf(1, "%11d %c ", size, isdir ? 'd' : '-');
	if (prefix) {
		if (prefix[0] && prefix[strlen(prefix)-1] != '/')
			sep = "/";
		else
			sep = "";
		fprintf(1, "%s%s", prefix, sep);
	}
	fprintf(1, "%s", name);
	if (flag[(int) 'F'] && isdir)
		fprintf(1, "/");
	fprintf(1, "\n");
}

void
usage(void)
{
	fprintf(1, "usage: ls [-dFl] [file...]\n");
	exit();
}

void
umain(int argc, char **argv)
{
	int i;
	struct Argstate args;

	argstart(&argc, argv, &args);
	while ((i = argnext(&args)) >= 0)
		switch (i) {
		case 'd':
		case 'F':
		case 'l':
			flag[i]++;
			break;
		default:
			usage();
		}

	if (argc == 1)
		ls("/", "");
	else {
		for (i = 1; i < argc; i++)
			ls(argv[i], argv[i]);
	}
}
