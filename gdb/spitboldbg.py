import gdb
import traceback


def display_regs():
    print("xl (rsi)", gdb.format_address(gdb.parse_and_eval("$rsi")).ljust(25), " ",
          "xr (rdi)", gdb.format_address(
              gdb.parse_and_eval("$rdi")).ljust(25), " ",
          "xt (rsi)", gdb.format_address(
              gdb.parse_and_eval("$rsi")).ljust(25), " ",
          "xs (rsp)", gdb.format_address(gdb.parse_and_eval("$rsp")).ljust(25))
    print("w0 (rax)", gdb.format_address(gdb.parse_and_eval("$rax")).ljust(25), " ",
          "wa (rcx)", gdb.format_address(
              gdb.parse_and_eval("$rcx")).ljust(25), " ",
          "wb (rbx)", gdb.format_address(
              gdb.parse_and_eval("$rbx")).ljust(25), " ",
          "wc (rdx)", gdb.format_address(gdb.parse_and_eval("$rdx")).ljust(25))
    print("ia (r12)", gdb.parse_and_eval("$r12").format_string().ljust(25), " ",
          "ra (xmms12)", gdb.parse_and_eval("$xmm12.v2_double").format_string().ljust(22), " ",
          "cp (r13)", gdb.format_address(gdb.parse_and_eval("$r13")).ljust(25), " ",
          "pc (rip)", gdb.format_address(gdb.parse_and_eval("$rip")).ljust(25), " ")


class DispSpitBlk(gdb.Command):
    def __init__(self):
        super().__init__("disp_spitblock", gdb.COMMAND_DATA)
    def invoke(self, arg, from_tty):
        print(arg)


class SpitRegs(gdb.Command):
    def __init__(self):
        super().__init__("spitregs", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        display_regs()


class DisplaySummary(gdb.Command):
    def __init__(self):
        super().__init__("display-spitbol-summary", gdb.COMMAND_DATA)

    def get_ptrval(self, name):
        return gdb.parse_and_eval(f"(void *){name}").format_string().ljust(18)

    def get_val(self, name):
        return gdb.parse_and_eval(f"{name}")

    def invoke(self, arg, from_tty):

        print("statb ", self.get_ptrval("statb"), " ",
              "state ", self.get_ptrval("state"), " ",
              "hshtb ", self.get_ptrval("hshtb"), " ",
              "hshte ", self.get_ptrval("hshte"))
        print("dnamb ", self.get_ptrval("dnamb"), " ",
              "dnamp ", self.get_ptrval("dnamp"), " ",
              "dname ", self.get_ptrval("dname"))
        print("stack ", self.get_ptrval("$rsp"), " ",
              "pmhbs ", self.get_ptrval("pmhbs"), " ",
              "compsp", self.get_ptrval("compsp"), " ",
              "highsp", self.get_ptrval("highsp"), " ",
              "lowsp ", self.get_ptrval("lowsp"), " ")
        print(" ")
        display_regs()


class DumpSpitbol(gdb.Command):
    def __init__(self):

        super().__init__("dump-spitbol-regions", gdb.COMMAND_DATA)

    def getregion(self, addr):
        if addr >= self.statb and addr < self.hshtb:
            return "varblks"
        elif addr >= self.hshtb and addr < self.hshte:
            return "hashtbl"
        elif addr >= self.hshte and addr < self.state:
            return "workarea"
        elif addr >= self.dnamb and addr < self.dnamp:
            return "dynamic"
        elif addr >= self.stack and addr < self.compsp:
            return "stack"
        else:
            return ""

    def getblkname(self, addr):
        if addr >= self.stack and addr < self.compsp:
            return "stack"
        s = gdb.format_address(gdb.parse_and_eval("*" + addr)).split()
        if len(s) == 1:
            return s[0]
        else:
            return s[1]

    def getrefblk(self, addr):
        if addr >= self.stack and addr < self.compsp:
            return "stack"
        s = gdb.format_address(addr).split()
        if len(s) == 1:
            return ""
        else:
            return s[1]

    def getrefblkx(self, addr):
        print(f">>> {addr}")
        if addr >= self.stack and addr < self.compsp:
            return "stack"
        s = gdb.format_address(addr).split()
        print(f">>>> {s}")
        if len(s) == 1:
            return ""
        else:
            return s[1]

    def scanregion(self, saddr, eaddr):
        current_address = saddr
        #print(f"Region: {self.getregion(saddr):8} {saddr}-{eaddr}")
        while current_address <= eaddr:
            data = current_address.dereference().cast(gdb.lookup_type("long long"))
            refregion = self.getregion(data)
            if refregion != "":
                if data.format_string() not in self.reftable[refregion]:
                    self.reftable[refregion][data.format_string()] = [
                        len(self.reftable[refregion])+1, set()]
                self.reftable[refregion][data.format_string()][1].add(
                    current_address.format_string())
#           print(current_address.format_string(), gdb.format_address(data), self.getregion(data))
            current_address += 1

    def invoke(self, arg, from_tty):
        self.addrtable = {}
        self.blockids = {}
        self.reftable = {
            "varblks": {},
            "hashtbl": {},
            "workarea": {},
            "dynamic": {},
            "stack": {}
        }
        printargs = arg.split()

        self.statb = gdb.parse_and_eval("(long long *)statb")
        self.state = gdb.parse_and_eval("(long long *)state")
        self.hshtb = gdb.parse_and_eval("(long long *)hshtb")
        self.hshte = gdb.parse_and_eval("(long long *)hshte")
        self.dnamb = gdb.parse_and_eval("(long long *)dnamb")
        self.dnamp = gdb.parse_and_eval("(long long *)dnamp")
        self.dname = gdb.parse_and_eval("(long long *)dname")
        self.stack = gdb.parse_and_eval("(long long *)highsp")
        self.pmhbs = gdb.parse_and_eval("(long long *)pmhbs")
        self.compsp = gdb.parse_and_eval("(long long *)compsp")
        self.lowsp = gdb.parse_and_eval("(long long *)lowsp")
        try:
            self.scanregion(self.statb, self.hshtb)
            self.scanregion(self.hshtb, self.hshte)
            self.scanregion(self.hshte, self.state)
            self.scanregion(self.dnamb, self.dnamp)
            self.scanregion(self.stack, self.compsp)

            addr2blk = {}
            addr2refs = {}
            # <<< debugging pluging >>>
            for i in self.reftable:
                for j in self.reftable[i]:
                    # s = []
                    addr2blk[j] = f"{self.getblkname(j)}_{self.reftable[i][j][0]}"
                    addr2refs[j] = self.reftable[i][j][1]
                    # for k in self.reftable[i][j][1]:
                    #    s.append(f"{k}:{self.getblkname(k)}")
                    # print(f"{i:8} {j}  {self.getblkname(j)}_{self.reftable[i][j][0]} {' '.join(s)}")

            for i in addr2refs:
                addr2refs[i] = list(addr2refs[i])
                for j in range(0, len(addr2refs[i])):
                    if addr2refs[i][j] in addr2blk:
                        addr2refs[i][j] = addr2blk[addr2refs[i][j]]

            if len(printargs) == 0 or "varblks" in printargs:
                print("")
                print("Varblks")
                current_address = self.statb
                while current_address < self.hshtb:
                    if current_address.format_string() in addr2blk:
                        r = ' '.join(
                            addr2refs[current_address.format_string()])
                        print(
                            f"\n{addr2blk[current_address.format_string()]} <== {r}")
                    data = current_address.dereference().cast(gdb.lookup_type("long long"))
                    s = ""
                    if data.format_string() in addr2blk:
                        s = addr2blk[data.format_string()]
                    print(
                        f"{current_address.format_string()} {data.format_string()} {self.getrefblk(data)} {s}")
                    current_address += 1

            if len(printargs) == 0 or "hashtable" in printargs:
                print("")
                print(f"Hashtable {self.hshtb}-{self.hshte}")
                current_address = self.hshtb
                while current_address < self.hshte:
                    data = current_address.dereference().cast(gdb.lookup_type("long long"))
                    if data == 0:
                        current_address += 1
                        continue
                    if current_address.format_string() in addr2blk:
                        r = ' '.join(
                            addr2refs[current_address.format_string()])
                        print(
                            f"\n{addr2blk[current_address.format_string()]} <== {r}")
                    s = ""
                    if data.format_string() in addr2blk:
                        s = addr2blk[data.format_string()]
                    print(
                        f"{current_address.format_string()} {data.format_string()} {self.getrefblk(data)} {s}")
                    current_address += 1

            if len(printargs) == 0 or "workarea" in printargs:
                print("")
                print("Workarea")
                current_address = self.hshte
                while current_address < self.state:
                    if current_address.format_string() in addr2blk:
                        r = ' '.join(
                            addr2refs[current_address.format_string()])
                        print(
                            f"\n{addr2blk[current_address.format_string()]} <== {r}")
                    data = current_address.dereference().cast(gdb.lookup_type("long long"))
                    s = ""
                    if data.format_string() in addr2blk:
                        s = addr2blk[data.format_string()]
                    print(
                        f"{current_address.format_string()} {data.format_string()} {self.getrefblk(data)} {s}")
                    current_address += 1

            if len(printargs) == 0 or "dynamic" in printargs:
                print("")
                print("Dynamic")
                current_address = self.dnamb
                while current_address < self.dnamp:
                    if current_address.format_string() in addr2blk:
                        r = ' '.join(
                            addr2refs[current_address.format_string()])
                        print(
                            f"\n{addr2blk[current_address.format_string()]} <== {r}")
                    data = current_address.dereference().cast(gdb.lookup_type("long long"))
                    s = ""
                    if data.format_string() in addr2blk:
                        s = addr2blk[data.format_string()]
                    print(
                        f"{current_address.format_string()} {data.format_string()} {self.getrefblk(data)} {s}")
                    current_address += 1

            if len(printargs) == 0 or "stack" in printargs:
                print("")
                print(f"Stack {self.stack}-{self.compsp}")
                current_address = self.stack
                while current_address < self.compsp:
                    if current_address.format_string() in addr2blk:
                        r = ' '.join(
                            addr2refs[current_address.format_string()])
                        print(
                            f"\n{addr2blk[current_address.format_string()]} <== {r}")
                    if current_address == self.pmhbs:
                        print("pmhbs")
                    data = current_address.dereference().cast(gdb.lookup_type("long long"))
                    s = ""
                    if data.format_string() in addr2blk:
                        s = addr2blk[data.format_string()]
                    print(
                        f"{current_address.format_string()} {data.format_string()} {self.getrefblk(data)} {s}")
                    current_address += 1

        # TODO: go through the reftable and get the actual name (should be just parsing the output of format_address
        #       then dump each region.  Note for the stack, want to "mark" where pmhbs points to.
        # later TODO: specific block formatting
        except Exception as e:
            print("-" * 60)
            traceback.print_exc()
            print("-" * 60)
            print("Exception: ", e)


SpitRegs()
DispSpitBlk()
DisplaySummary()
DumpSpitbol()
