# Creating a release

1. **Version**  
   Optionally bump `VersionName` in `Uiana/Uiana.uplugin` (e.g. `1.05` → `1.06`).

2. **Tag and push**  
   Use a tag that starts with `v` (e.g. match the plugin version):
   ```bash
   git tag v1.05
   git push origin v1.05
   ```

3. **Automated release**  
   The [Release workflow](.github/workflows/release.yml) runs on tag push. It:
   - Zips the `Uiana` folder (so users get a single **Uiana** folder to drop into Plugins).
   - Creates a GitHub Release for that tag with the zip attached and generated release notes.

4. **Manual release (optional)**  
   You can also [create a release on GitHub](https://github.com/djhaled/Uiana-MapImporter/releases/new), create the tag there, and upload a zip of the `Uiana` folder yourself.
