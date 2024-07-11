{
  description = "python packages test";

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        makePythonPath = with pkgs; drvs: lib.makeSearchPath python311.sitePackages drvs;
      in
      {
        devShells.default = pkgs.mkShell rec {
          #venvDir = "venv";
          packages =
            with pkgs;
            [ python311 ]
            ++ (with pkgs.python311Packages; [
              pip
              #venvShellHook
              flask
              sqlitedict
              numpy
              matplotlib
              pandas
              pandas-stubs
            ]);
          shellHook = ''
            #export PYTHONPATH=$PYTHONPATH:${makePythonPath packages}
            exec zsh
          '';
        };
      }
    );
}
