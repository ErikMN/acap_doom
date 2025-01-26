/* Collection of utility functions */

export function capitalizeFirstLetter(word: string): string {
  if (!word) return word;
  return word.charAt(0).toUpperCase() + word.slice(1);
}

/**
 * Converts CamelCase or PascalCase strings into a "nice name" with spaces,
 * preserving chunks of consecutive uppercase letters at the start.
 *
 * Examples:
 *   "MaxAlarmThreshold" --> "Max alarm threshold"
 *   "BGTransparency"    --> "BG transparency"
 *   "HTTPRequestId"     --> "HTTP request id"
 */
export function toNiceName(word: string): string {
  if (!word) return word;

  // 1) Split the word into tokens:
  //    - One or more uppercase letters in a row (e.g. "BG", "HTTP")
  //      if at the start or just before a lowercased letter,
  //    - OR one uppercase letter optionally followed by lowercase letters,
  //    - OR digits.
  //
  // Example: "BGTransparency" => ["BG", "Transparency"]
  //          "MaxAlarmThreshold" => ["Max", "Alarm", "Threshold"]
  //          "HTTPRequestId" => ["HTTP", "Request", "Id"]
  const tokens = word.match(/[A-Z]+(?![a-z])|[A-Z]?[a-z]+|\d+/g) || [word];

  // 2) Convert the tokens so that:
  //    - If a token is multiple uppercase letters (e.g. "BG", "HTTP") and it's the *first* token, keep it uppercase.
  //    - Otherwise, lowercase it entirely.
  // This yields e.g. ["BG", "transparency"], ["max", "alarm", "threshold"], ["HTTP", "request", "id"]
  const processedTokens = tokens.map((token, index) => {
    // Check if it's multiple uppercase letters
    if (/^[A-Z]{2,}$/.test(token)) {
      // If it's the *first* token, keep it uppercase (e.g. "BG")
      return index === 0 ? token : token.toLowerCase();
    }
    // Otherwise, make the token all lowercase (e.g. "Transparency" => "transparency")
    return token.toLowerCase();
  });

  // 3) Join them with a space: "BG transparency", "max alarm threshold", etc.
  const joined = processedTokens.join(' ');

  // 4) Capitalize the first letter of the entire phrase:
  //    "BG transparency" -> "BG transparency" (first letter is 'B', uppercase)
  //    "max alarm threshold" -> "Max alarm threshold"
  return joined.charAt(0).toUpperCase() + joined.slice(1);
}

/* Play a sound at volume */
export const playSound = (url: string, volume: number = 0.5) => {
  const isMuted = localStorage.getItem('mute');
  if (isMuted === 'true') {
    return;
  }
  const audio = new Audio(url);
  audio.volume = Math.min(Math.max(volume, 0), 1);
  audio.play().catch((err) => {
    console.warn('Failed to play sound:', err);
  });
};
