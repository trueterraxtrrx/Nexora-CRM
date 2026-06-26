/** @type {import('tailwindcss').Config} */
export default {
  content: ['./index.html', './src/**/*.{js,ts,jsx,tsx}'],
  theme: {
    extend: {
      colors: {
        brand: { DEFAULT: '#6366f1', hover: '#818cf8', muted: '#4f46e5' },
        ink: '#e5edf6',
        bg: '#0f1722',
        surface: '#141c29',
        panel: '#18212f',
        line: '#2b3748',
        signal: '#2dd4bf',
        warn: '#f59e0b',
        danger: '#fb7185',
      },
      fontFamily: { sans: ['Inter', 'system-ui', 'sans-serif'] },
    },
  },
  plugins: [],
}
