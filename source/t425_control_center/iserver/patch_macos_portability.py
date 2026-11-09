from pathlib import Path
import sys
root=Path(sys.argv[1])
server=root/"server.h"
t=server.read_text()
start="/* machine specific stuff  */"
end="#ifndef HOST_ID"
if "F296_MACOS" not in t:
    a=t.index(start)
    b=t.index(end, a)
    t=t[:a]+"/* F296 macOS host definitions */\n#ifdef F296_MACOS\n#include \"macos.h\"\n#else\n"+t[a:b]+"#endif /* F296_MACOS */\n\n"+t[b:]
    server.write_text(t)

p=root/"sh.c"
s=p.read_text()
s=s.replace("static int debug;", "int debug;")
p.write_text(s)
p=root/"record.c"
s=p.read_text().replace("#include <malloc.h>", "#include <stdlib.h>")
p.write_text(s)
p=root/"sh.c"
s=p.read_text()
s=s.replace("static int debug;", "int debug;")
s=s.replace("static int LinkValid=1;", "static int LinkValid=1;\nstatic int execute();\nstatic int initcmds();")
s=s.replace("   char  *malloc();\n", "")
s=s.replace("   char  *getenv();\n", "")
p.write_text(s)
p=root/"filec.c"
s=p.read_text().replace("   extern int  sys_nerr;\n   extern char *sys_errlist[];\n", "")
p.write_text(s)
