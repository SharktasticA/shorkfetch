{
    description = "A cute, shark-themed Linux fetch tool that displays system, hardware, and environment information in a summarised format";
    inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    inputs.self.submodules = true;

    outputs = { nixpkgs, ... }:
    {
        packages = nixpkgs.lib.genAttrs [ "x86_64-linux" "aarch64-linux" "i686-linux" ]
        (system:
            let pkgs = nixpkgs.legacyPackages.${system}; in
            {
                default = pkgs.stdenv.mkDerivation
                {
                    name = "shorkfetch";
                    src = ./.;

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
