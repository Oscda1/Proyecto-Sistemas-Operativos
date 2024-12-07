{ pkgs }: {
	deps = [
   pkgs.graphviz
   pkgs.ncurses6
   pkgs.rubyPackages_3_1.ncursesw
   pkgs.ncurses
   pkgs.speedtest-cli
   pkgs.unixtools.ping
   pkgs.nano
   pkgs.vim
		pkgs.clang
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
	];
}