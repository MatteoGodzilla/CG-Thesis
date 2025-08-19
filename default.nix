with import <nixpkgs> {};

stdenv.mkDerivation {
    name = "opengl-thesis";
    system = builtins.currentsystem;
    src = ./.;
    buildInputs = [ glfw ];
}
