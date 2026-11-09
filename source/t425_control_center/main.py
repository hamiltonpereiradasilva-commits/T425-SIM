import tkinter as tk
from gui.app import ControlCenter

def main():
    root = tk.Tk()
    ControlCenter(root)
    root.mainloop()

if __name__ == "__main__":
    main()
