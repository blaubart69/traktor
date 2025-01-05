import { defineBoot } from '#q-app/wrappers';
import axios, { type AxiosInstance } from 'axios';

declare module 'vue' {
  interface ComponentCustomProperties {
    $axios: AxiosInstance;
    $api: AxiosInstance;
  }
}

// Be careful when using SSR for cross-request state pollution
// due to creating a Singleton instance here;
// If any client changes this (global) instance, it might be a
// good idea to move this instance creation inside of the
// "export default () => {}" function below (which runs individually
// for each client)
const api = axios.create({ baseURL: '/api' });

export default defineBoot(({ app }) => {
  // for use inside Vue files (Options API) through this.$axios and this.$api
  // api.defaults.headers.common['Access-Control-Allow-Origin'] = 'http://localhost'
  // api.defaults.headers.common['Access-Control-Allow-Methods'] = 'GET, POST, OPTIONS'
  // api.defaults.headers.common['Access-Control-Allow-Headers'] = 'Content-Type'
  // api.defaults.headers.common['Accept'] = 'application/json,*/*'
  // api.defaults.headers.common['Content-Type'] = '*/*'
  // api.defaults.headers.common['Host'] = '127.0.0.1'

  app.config.globalProperties.$axios = axios;
  // ^ ^ ^ this will allow you to use this.$axios (for Vue Options API form)
  //       so you won't necessarily have to import axios in each vue file

  app.config.globalProperties.$api = api;
  // ^ ^ ^ this will allow you to use this.$api (for Vue Options API form)
  //       so you can easily perform requests against your app's API
});

export { api };
