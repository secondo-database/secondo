// Which labels get drawn when more of them want the same piece of screen than
// will fit. Overlapping map labels are unreadable, and a map looks uncluttered
// mostly because of the labels it decides not to draw.
//
// This was deck's CollisionFilterExtension first, which does the same job on
// the GPU: it renders every label's box into an offscreen map, and each label
// checks whether it can still see itself there. It fails closed. When deck's
// devicePixelRatio bookkeeping and the actual drawing buffer disagree -- which
// is the normal state of affairs on a HiDPI display, and which a light/dark
// switch was enough to provoke -- every label samples the wrong pixel, fails to
// find itself, and hides; they all stay gone until a pan or zoom happens to
// rebuild the offscreen map. Losing every label to a canvas resize is much
// worse than the clutter the filter was there to prevent.
//
// So the test runs here instead, on plain rectangles in screen pixels.

// Kept in step with LABEL_TEXT in MapView: the boxes reserved here have to be
// the boxes deck goes on to draw.
export const LABEL_FONT_FAMILY = "ui-sans-serif, system-ui, sans-serif";
export const LABEL_FONT_WEIGHT = 600;
export const LABEL_FONT_SIZE_PX = 12;

// Half a line of 12px text, plus the air a label needs around it so two
// survivors never end up flush against one another.
const HALF_LINE_PX = 7;
const PADDING_PX = 3;

let measureCtx: CanvasRenderingContext2D | null | undefined;

function context(): CanvasRenderingContext2D | null {
  if (measureCtx === undefined) {
    measureCtx = document.createElement("canvas").getContext("2d");
    if (measureCtx) {
      measureCtx.font = `${LABEL_FONT_WEIGHT} ${LABEL_FONT_SIZE_PX}px ${LABEL_FONT_FAMILY}`;
    }
  }
  return measureCtx;
}

const widths = new Map<string, number>();

/** How wide `text` will be on screen, in pixels, in the label font. */
export function labelWidthPx(text: string): number {
  const cached = widths.get(text);
  if (cached !== undefined) return cached;
  const ctx = context();
  // Without a 2D context (a headless test environment) fall back to an average
  // advance width. Decluttering then places labels a little loosely rather than
  // wrongly, which is the harmless direction to be wrong in.
  const w = ctx ? ctx.measureText(text).width : text.length * 6.6;
  widths.set(text, w);
  return w;
}

type Box = [number, number, number, number];

/**
 * The labels placed so far this frame. One placer is shared by every layer, so
 * a layer's labels give way to another layer's and not only to their own.
 *
 * Labels are offered in draw order and the first to ask for a piece of the map
 * keeps it, which makes the survivors stable from frame to frame rather than
 * flickering between equals.
 *
 * The scan is linear in the labels already kept, so the whole pass is O(n*k).
 * That stays cheap while the labels compete for space, because k is then only
 * however many fit on the screen: berlintest's largest relation, Landstrassen
 * at 1139 features, measures ~4 ms a frame fitted and ~7 ms zoomed in. It
 * degrades when a very large relation is zoomed into far enough that nothing
 * overlaps any more and k approaches n -- 10k features that way costs ~550 ms.
 * A grid would fix that, but by then deck is drawing ten thousand labels and
 * nine times that many haloed glyph runs, so the map has lost either way.
 */
export class LabelPlacer {
  private readonly placed: Box[] = [];

  /**
   * Reserve the box for `text` centred at (x, y) in screen pixels. Returns
   * false, reserving nothing, when it would touch a label already placed --
   * that label is not drawn.
   */
  place(text: string, x: number, y: number): boolean {
    const halfW = labelWidthPx(text) / 2 + PADDING_PX;
    const halfH = HALF_LINE_PX + PADDING_PX;
    const box: Box = [x - halfW, y - halfH, x + halfW, y + halfH];
    for (const b of this.placed) {
      if (box[0] < b[2] && b[0] < box[2] && box[1] < b[3] && b[1] < box[3]) {
        return false;
      }
    }
    this.placed.push(box);
    return true;
  }
}
