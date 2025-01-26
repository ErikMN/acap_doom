/* Application settings JSON interface */
export interface AppSettings {
  debug: boolean;
  wsDefault: boolean;
}

/* Application settings default values */
export const defaultAppSettings: AppSettings = {
  debug: false,
  wsDefault: true
};

/* VideoBox dimensions interface */
export interface Dimensions {
  videoWidth: number; // Video stream width
  videoHeight: number; // Video stream height
  pixelWidth: number; // Video pixel width
  pixelHeight: number; // Video pixel height
  offsetX: number; // Offset X (left margin of the video in the container)
  offsetY: number; // Offset Y (top margin of the video in the container)
}
