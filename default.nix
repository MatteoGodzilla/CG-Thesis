with import <nixpkgs> {};

stdenv.mkDerivation {
    name = "opengl-thesis";
    system = builtins.currentsystem;
    src = ./.;
    buildInputs = [ 
        glfw
        xorg.libX11
        xorg.libXrandr
        wayland
        kdePackages.kdialog
    ];
}
