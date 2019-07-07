import Vue from "vue";
import App from "./App.vue";
import Vuex from "vuex";
import Router from "vue-router";
import { router } from "./router";
import "material-icons/iconfont/material-icons.css";
import "vuetify/dist/vuetify.css";
import store from "./store";
import Vuetify from "vuetify";

Vue.use(Vuex);
Vue.use(Router);
Vue.use(Vuetify,
  {
  theme: {
    primary: "#2191C3",
    secondary: "#6B3B3B",
    accent: "#F13040",
    error: "#f44336",
    info: "#222128",
    success: "#64a764",
    warning: "#e1972c",
  },
});


Vue.config.productionTip = false;

new Vue({
  router,
  store,
  render: (h) => h(App),
}).$mount("#app");
