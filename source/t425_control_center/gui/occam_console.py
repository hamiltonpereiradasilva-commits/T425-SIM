"""Low-overhead Tk Listbox console for real IServer process output."""
from pathlib import Path
from tkinter import ttk, filedialog
import tkinter as tk


class OccamConsole(ttk.Frame):
    MAX_ROWS = 20000

    def __init__(self, master):
        super().__init__(master)
        toolbar = ttk.Frame(self)
        toolbar.pack(fill="x", pady=(0, 5))
        ttk.Button(toolbar, text="Limpar", command=self.clear).pack(side="left")
        ttk.Button(toolbar, text="Copiar tudo", command=self.copy_all).pack(side="left", padx=4)
        ttk.Button(toolbar, text="Salvar TXT…", command=self.save_dialog).pack(side="left")
        ttk.Label(toolbar, text="Exibição literal do stdout/stderr recebido do IServer; sem saída simulada.").pack(side="right")

        body = ttk.Frame(self)
        body.pack(fill="both", expand=True)
        self.listbox = tk.Listbox(
            body, height=8, activestyle="none", exportselection=False,
            background="#111827", foreground="#d1fae5",
            selectbackground="#2563eb", selectforeground="#ffffff",
            font=("Menlo", 12), relief="flat", borderwidth=0,
        )
        self.vbar = ttk.Scrollbar(body, orient="vertical", command=self.listbox.yview)
        self.hbar = ttk.Scrollbar(self, orient="horizontal", command=self.listbox.xview)
        self.listbox.configure(yscrollcommand=self.vbar.set, xscrollcommand=self.hbar.set)
        self.listbox.pack(side="left", fill="both", expand=True)
        self.vbar.pack(side="right", fill="y")
        self.hbar.pack(fill="x")
        self._stream_rows = {"stdout": None, "stderr": None}
        self._stream_pending = {"stdout": "", "stderr": ""}
        self._stream_first_line = {"stdout": True, "stderr": True}
        self._pending_cr = {"stdout": False, "stderr": False}

    def append_text(self, stream, text):
        """Append decoded bytes from one stream, keeping each stream's order.

        Newlines become visual Listbox rows. Partial lines remain visible and are
        updated in place as later chunks arrive. Raw bytes are retained by the
        runner's JSONL events in base64.
        """
        if stream not in self._stream_rows or not text:
            return
        # Treat CR/LF as a visual line boundary without changing the raw event log.
        if self._pending_cr[stream]:
            text = "\n" + (text[1:] if text.startswith("\n") else text)
            self._pending_cr[stream] = False
        if text.endswith("\r"):
            text = text[:-1]
            self._pending_cr[stream] = True
        pending = self._stream_pending[stream] + text
        pending = pending.replace("\r\n", "\n").replace("\r", "\n")
        pieces = pending.split("\n")
        for complete in pieces[:-1]:
            line = complete
            self._render_line(stream, line, complete=True)
        tail = pieces[-1]
        self._stream_pending[stream] = tail
        if tail:
            self._render_line(stream, tail, complete=False)
        self._trim()

    def flush_stream(self, stream):
        pending = self._stream_pending.get(stream, "")
        if pending or self._pending_cr.get(stream):
            self._render_line(stream, pending, complete=True)
            self._stream_pending[stream] = ""
            self._pending_cr[stream] = False

    def _render_line(self, stream, content, complete):
        row = self._stream_rows[stream]
        marker = "[stdout] " if stream == "stdout" else "[stderr] "
        value = marker + content
        color = "#d1fae5" if stream == "stdout" else "#fca5a5"
        if row is None:
            self.listbox.insert("end", value)
            row = self.listbox.size() - 1
            self._stream_rows[stream] = row
            self.listbox.itemconfigure(row, foreground=color)
        else:
            self.listbox.delete(row)
            self.listbox.insert(row, value)
            self.listbox.itemconfigure(row, foreground=color)
        if complete:
            self._stream_rows[stream] = None
            self._stream_first_line[stream] = False
            self.listbox.see("end")
        else:
            self.listbox.see("end")

    def _trim(self):
        excess = self.listbox.size() - self.MAX_ROWS
        if excess <= 0:
            return
        self.listbox.delete(0, excess - 1)
        for stream, row in self._stream_rows.items():
            if row is not None:
                self._stream_rows[stream] = row - excess if row >= excess else None

    def clear(self):
        self.listbox.delete(0, "end")
        for stream in self._stream_rows:
            self._stream_rows[stream] = None
            self._stream_pending[stream] = ""
            self._stream_first_line[stream] = True
            self._pending_cr[stream] = False

    def copy_all(self):
        text = "\n".join(self.listbox.get(0, "end"))
        self.clipboard_clear()
        self.clipboard_append(text)
        return text

    def save_dialog(self):
        path = filedialog.asksaveasfilename(
            defaultextension=".txt", filetypes=[("Texto", "*.txt")]
        )
        if path:
            self.save(path)

    def save(self, path):
        Path(path).write_text("\n".join(self.listbox.get(0, "end")) + "\n", encoding="utf-8")
