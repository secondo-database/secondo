import Vue from "vue";
import Vuex from "vuex";
import journal from "./journal";
import shared from "./shared";
import workerStatus from "./workerstatus";
import config from "./config";

Vue.use(Vuex);

const store = new Vuex.Store({
  modules: {
    shared,
    journal,
    workerStatus,
    config,
  },
});

export default store;
