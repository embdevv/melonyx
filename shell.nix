{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "melonyx-dev";

  nativeBuildInputs = with pkgs; [
    cmake
    gcc
    pkg-config
    wayland-scanner
    (python3.withPackages (ps: with ps; [ jinja2 ]))
  ];

  buildInputs = with pkgs; [
    glfw
    libGL
    xorg.libX11
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
    wayland
    wayland-protocols
    libxkbcommon
    libffi
  ];

  shellHook = ''
    export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath [
      pkgs.libGL
      pkgs.xorg.libX11
      pkgs.xorg.libXrandr
      pkgs.xorg.libXinerama
      pkgs.xorg.libXcursor
      pkgs.xorg.libXi
      pkgs.wayland
      pkgs.libxkbcommon
    ]}:$LD_LIBRARY_PATH"
    echo "melonyx dev shell ready — cmake $(cmake --version | head -n1)"
  '';
}