{
    description = "A cute, shark-themed Linux fetch tool that displays system, hardware, and environment information in a summarised format";
    inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    inputs.shorkcommon = { url = "github:SharktasticA/shorkcommon"; flake = false; };
    inputs.cpuinfo-ds = { url = "github:SharktasticA/cpuinfo-ds"; flake = false; };

    outputs = { self, nixpkgs, shorkcommon, cpuinfo-ds, ... }:
    {
        packages = nixpkgs.lib.genAttrs [ "x86_64-linux" "aarch64-linux" "i686-linux" ]
        (system:
            let pkgs = nixpkgs.legacyPackages.${system}; in
            {
                default = pkgs.stdenv.mkDerivation
                {
                    name = "shorkfetch";
                    src = ./.;

                    postPatch = ''
                        printf '#!/bin/sh\nexit 0\n' > sync-submodules.sh
                        chmod +x sync-submodules.sh
                        rm -rf shorkcommon cpuinfo-ds
                        cp -r ${shorkcommon} shorkcommon
                        cp -r ${cpuinfo-ds} cpuinfo-ds
                        chmod -R u+w shorkcommon cpuinfo-ds
                    '';

                    nativeBuildInputs = [
                        pkgs.gcc
                        pkgs.gnumake
                    ];

                    buildInputs = [
                        pkgs.glibc
                    ];

                    makeFlags = [ "PREFIX=$(out)" ];
                };
            }
        );
    };
}
