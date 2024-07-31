module.exports = {
  parserOptions: {
    ecmaVersion: 2022, // Specify the ECMAScript version
    sourceType: 'module', // Allow use of import/export statements
    ecmaFeatures: {
      jsx: true // Enable JSX parsing
    }
  },
  settings: {
    // https://github.com/jsx-eslint/eslint-plugin-react#configuration
    react: {
      version: 'detect'
    }
  },
  extends: ['eslint:recommended', 'plugin:react/recommended', 'prettier'],
  plugins: ['react', 'react-hooks'],
  rules: {
    'no-unused-vars': 'warn',
    'react/prop-types': 'warn', // Warn if component is missing PropTypes
    'react-hooks/rules-of-hooks': 'error', // Enforce rules of Hooks
    'react-hooks/exhaustive-deps': 'warn' // Check effect dependencies
  },
  env: {
    browser: true,
    node: true,
    es6: true // Enable ES6 global variables and syntax
  }
};
