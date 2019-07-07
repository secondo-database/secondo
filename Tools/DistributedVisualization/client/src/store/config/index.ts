import {
    SET_FLIP_MODE,
} from "../mutation_types";
import store from "..";
const FLIP_MODE_KEY: string = "secondo.flip.mode";
export default {
    state: {
        flipMode: "horizontal",
    },
    mutations: {
        [SET_FLIP_MODE](state: any, mode: string) {
            state.flipMode = mode;
        },
    },
    actions: {
        switchFlipMode({ commit }: any) {
            const mode: string | null = localStorage.getItem(FLIP_MODE_KEY);
            const flipped: string = getFlippedMode(mode);
            localStorage.setItem(FLIP_MODE_KEY, flipped);
            commit(SET_FLIP_MODE, flipped);
        },
        loadFlipMode({ commit }: any) {
            const mode: string | null = localStorage.getItem(FLIP_MODE_KEY);
            if (mode !== null) {
                commit(SET_FLIP_MODE, mode);
            }

        },
    },
    getters: {
        flipMode(state: any) {
            return state.flipMode;
        },
    },
};

function getFlippedMode(mode: string | null): string {
    let flipped: string = "horizontal";
    if (mode === null) {

        if (store.getters.flipMode === "horizontal") {
            flipped = "vertical";
        } else {
            flipped = "horizontal";
        }
    } else {
        if (mode === "horizontal") {
            flipped = "vertical";
        } else {
            flipped = "horizontal";
        }
    }
    return flipped;
}
