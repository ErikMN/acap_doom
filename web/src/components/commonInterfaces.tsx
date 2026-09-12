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
