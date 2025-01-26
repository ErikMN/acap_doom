/// <reference types="vite/client" />
/// <reference types="vite-plugin-svgr/client" />
/**
 * Declare a module for files with the `.oga` extension.
 * This is necessary because TypeScript does not natively understand
 * non-code assets like audio files.
 *
 * When you import an `.oga` file, TypeScript will treat it as a module
 * that exports a string (typically the URL of the asset after being processed by Vite).
 */
declare module '*.oga' {
  /**
   * The imported value is of type `string`.
   * This string usually represents the URL or file path of the `.oga` asset.
   */
  const value: string;

  /**
   * Export the value as the default export of the module.
   * This enables usage like:
   *
   * import lockSoundUrl from './path/to/sound.oga';
   */
  export default value;
}
