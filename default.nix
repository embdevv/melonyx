{ pkgs ? import <nixpkgs> {} }:

let
  mingwPkgs = pkgs.pkgsCross.mingwW64;
in
mingwPkgs.stdenv.mkDerivation {
  pname = "melonyx";
  version = "1.0.0";
  src = ./.;

  nativeBuildInputs = [ pkgs.cmake pkgs.zip ];
  
  # Ensure these are the mingw-specific versions
  buildInputs = [ 
    mingwPkgs.glfw 
    #mingwPkgs.glm 
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    "-DCMAKE_PREFIX_PATH=${mingwPkgs.glfw}"
  ];

  postInstall = ''
    mkdir -p $out/zip
    zip -j $out/zip/release.zip $out/bin/*.exe
  '';
}