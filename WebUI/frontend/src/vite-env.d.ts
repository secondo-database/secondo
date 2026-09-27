// Vite inlines `?raw` imports as strings. Declared narrowly rather than by
// referencing all of vite/client, so no other ambient module types leak in.
declare module "*.svg?raw" {
  const src: string;
  export default src;
}

// Stylesheets are imported only for their side effect. TypeScript 6+ checks
// side-effect imports (noUncheckedSideEffectImports), so they need a module.
declare module "*.css" {}
