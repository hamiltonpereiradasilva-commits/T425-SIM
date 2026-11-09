from pathlib import Path
import sys
p=Path(sys.argv[1])
s=p.read_text(errors="replace")
anchor='''  if (buffer_depth < 1) {
    (void) sprintf (lnkops_errorstring, " : module [linkops.c], function [OPS_Open]\\n -> bad buffer depth value\\n");
    OPSerrorstatus = STATUS_BAD_OPERATION;
    return (OPSerrorstatus);
  }
'''
if 'F296 direct LinkUSB LinkIO route' not in s:
    if anchor not in s:
        raise SystemExit("could not locate OPS_Open insertion point")
    block=anchor+"""
  /* F296 direct LinkUSB LinkIO route; all subsequent original OPS/LOPS/SP calls remain. */
  if (resource != NULL && strcmp(resource, "linkusb0") == 0) {
    extern int METAsearch_database;
    extern int METAmethod;
    extern int OpenLink();
    METAsearch_database = 0;
    METAmethod = 0;
    *connection_id = (long int) OpenLink("linkusb0");
    METAsearch_database = 1;
    if (*connection_id < 0) {
      (void) sprintf(lnkops_errorstring, "LinkUSB Link0 open failed\\n");
      OPSerrorstatus = STATUS_LINK_FATAL;
      return OPSerrorstatus;
    }
    if (localmethod) *localmethod = 0;
    if (link_name) (void) strcpy(link_name, "linkusb0");
    if (server_name) (void) strcpy(server_name, "localhost");
    OPScommsmode = COMMSsynchronous;
    OPSerrorflagmode = ERRORignore;
    OPSerrorstatus = STATUS_NOERROR;
    OPSmethod = OPSMETHODlinkio;
    return OPSerrorstatus;
  }
"""
    s=s.replace(anchor,block,1)
    p.write_text(s)
