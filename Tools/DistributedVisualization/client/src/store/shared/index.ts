import {
  CLEAR_GLOBAL_ERROR,
  SET_GLOBAL_ERROR,
  SET_GLOBAL_LOADING,
} from "../mutation_types";

export default {
  state: {
    loading: false,
    error: undefined,
  },
  mutations: {
    [SET_GLOBAL_LOADING](state: any, payload: boolean) {
      state.loading = payload;
    },
    [SET_GLOBAL_ERROR](state: any, payload: string) {
      state.error = payload;
    },
    [CLEAR_GLOBAL_ERROR](state: any) {
      state.error = undefined;
    },
  },
  actions: {
    setGlobalLoadingAction({ commit }: any, payload: any) {
      commit(SET_GLOBAL_LOADING, payload);
    },
    setGlobalErrorAction({ commit }: any, payload: any) {
      commit(SET_GLOBAL_ERROR, payload);
    },
    clearGlobalErrorAction({ commit }: any) {
      commit(CLEAR_GLOBAL_ERROR);
    },
  },
  getters: {
    loading(state: any) {
      return state.loading;
    },
    error(state: any) {
      return state.error;
    },
  },
};
