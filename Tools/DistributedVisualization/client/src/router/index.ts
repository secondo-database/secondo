import Vue from "vue";
import VueRouter, { RouteConfig } from "vue-router";
import Journal from "../components/Journal.vue";

Vue.use(VueRouter);


const routes: RouteConfig[] = [
  {
    path: "/journal",
    name: "Distributed jobs",
    component: Journal,
  },
  {
    path: "/",
    name: "Distributed jobs",
    component: Journal,
  },
];


export const router = new VueRouter({
  routes,
});
