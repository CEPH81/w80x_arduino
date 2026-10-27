// Carga por CK-Link con XuanTie (T-Head) Debugger Server.
//
//	wm_img cklink -file sketch.ck.bin -chip w800 [-server DebugServerConsole] [-algo algoritmo.elf]
//
// Escribe el archivo en la flash a partir de 0x08002000 con el algoritmo de
// flash de WinnerMicro que trae el Debugger Server, y reinicia el chip.
package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
)

func serverDirs() []string {
	var d []string
	if v := os.Getenv("XUANTIE_DEBUGSERVER"); v != "" {
		d = append(d, v)
	}
	switch runtime.GOOS {
	case "windows":
		for _, pf := range []string{os.Getenv("ProgramFiles(x86)"), os.Getenv("ProgramFiles"), `C:\`} {
			if pf == "" {
				continue
			}
			for _, n := range []string{"T-Head DebugServer", "C-Sky", "XuanTie DebugServer", "T-HEAD DebugServer"} {
				d = append(d, filepath.Join(pf, n))
			}
		}
	default:
		home, _ := os.UserHomeDir()
		d = append(d, "/usr/bin", "/usr/local/bin", "/opt/t-head-debugserver", "/opt/T-HEAD_DebugServer",
			filepath.Join(home, "T-HEAD_DebugServer"), filepath.Join(home, "t-head-debugserver"))
	}
	return d
}

func findServer() string {
	exe := "DebugServerConsole"
	if runtime.GOOS == "windows" {
		exe += ".exe"
	}
	if p, err := exec.LookPath(exe); err == nil {
		return p
	}
	for _, d := range serverDirs() {
		var found string
		filepath.Walk(d, func(p string, info os.FileInfo, err error) error {
			if err == nil && info != nil && found == "" && !info.IsDir() && strings.EqualFold(info.Name(), exe) {
				found = p
			}
			return nil
		})
		if found != "" {
			return found
		}
	}
	return ""
}

// algoritmo de flash: W806 tiene el suyo; W800/W801/W802 usan el de W800
func findAlgo(server, chip string) string {
	want := "W800"
	if strings.HasPrefix(strings.ToLower(chip), "w806") {
		want = "W806"
	}
	var found string
	filepath.Walk(filepath.Dir(server), func(p string, info os.FileInfo, err error) error {
		if err != nil || info == nil {
			return nil
		}
		n := strings.ToUpper(info.Name())
		if found == "" && !info.IsDir() && strings.HasSuffix(n, ".ELF") &&
			strings.Contains(n, want) && strings.Contains(n, "FLASH") {
			found = p
		}
		return nil
	})
	return found
}

func cklinkMain(args []string) {
	fs := flag.NewFlagSet("cklink", flag.ExitOnError)
	file := fs.String("file", "", "archivo .ck.bin generado con -ck")
	chip := fs.String("chip", "w800", "chip: w800, w801s, w802, w806")
	server := fs.String("server", "", "ruta de DebugServerConsole (si no, se busca)")
	algo := fs.String("algo", "", "algoritmo de flash .elf (si no, se busca junto al servidor)")
	fs.Parse(args)
	if *file == "" {
		fs.Usage()
		os.Exit(2)
	}
	if *server == "" {
		*server = findServer()
	}
	if *server == "" {
		fail("no se encuentra DebugServerConsole. Instale XuanTie (T-Head) Debugger Server " +
			"o indique su carpeta en la variable XUANTIE_DEBUGSERVER")
	}
	if *algo == "" {
		*algo = findAlgo(*server, *chip)
	}
	if *algo == "" {
		fail("no se encuentra el algoritmo de flash de WinnerMicro (%s*FLASH*.elf) junto a %s; "+
			"indique su ruta con -algo", strings.ToUpper(*chip)[:4], *server)
	}
	abs, _ := filepath.Abs(*file)
	script := filepath.Join(os.TempDir(), "w80x_cklink.txt")
	cmds := fmt.Sprintf("flash program -f \"%s\" -b -a 0x%08X\nreset\nquit\n", abs, ckBase)
	if err := os.WriteFile(script, []byte(cmds), 0644); err != nil {
		fail("%v", err)
	}
	fmt.Printf("CK-Link: %s\nAlgoritmo: %s\n", *server, *algo)
	cmd := exec.Command(*server, "-prereset", "-flash-algorithm", *algo, "-cmd-script", script, "-skip-enter")
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Stdin = os.Stdin
	if err := cmd.Run(); err != nil {
		fail("el Debugger Server termino con error: %v", err)
	}
}
