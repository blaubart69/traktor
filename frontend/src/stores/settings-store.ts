import { defineStore } from 'pinia';
import { api } from '../boot/axios';
import type { HarrowSettings } from '../components/models';

export function createSettingsState() {
  return {
    records: [] as string[],
  };
}

export const useSettingsStore = defineStore('SettingsStore', {
  state: () => {
    return createSettingsState();
  },
  getters: {
    profiles: (state) => Object.values(state.records).sort((a, b) => (a < b ? -1 : 1)),
  },
  actions: {
    async list() {
      const { data } = await api.get('/list');
      this.records = data.entries as string[];
      const index = this.records.indexOf('lastSettings.json');
      this.records.splice(index, 1);
      return this.records;
    },
    async current() {
      const { data } = await api.get('/current');
      return data as HarrowSettings;
    },
    async load(path: string) {
      const { data } = await api.get(`/load/${path}`);
      return data;
    },
    async save(path: string, settings: HarrowSettings) {
      const index = this.records.findIndex((r) => r === path);
      console.log('save: ', index, path, this.records);
      const { data } = await api.post(`/save/${path}`, settings);
      if (index >= 0) {
        this.records[index] = path;
      } else {
        this.records.push(path);
      }
      return data;
    },
    async applyChanges(settings: HarrowSettings) {
      const { data } = await api.post('/applyChanges', settings);
      return data;
    },
  },
});
