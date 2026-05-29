{
    description = "A cute shark-themed Linux fetch tool designed with speed, consistent output and vintage PC CPU & GPU support in mind";
    inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
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
                        pkgs.glibc.static 
                    ];

                    makeFlags = [ "PREFIX=$(out)" ];
                };
            }
        );
    };
}
