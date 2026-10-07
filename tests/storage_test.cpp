<path>tests/storage_test.cpp</path>
<type>file</type>
<content>
1: #include <iostream>
2: #include <fstream>
3: #include <cstring>
4: #include <cstdio>
5: #include <string>
6: #include "../src/storage/block_list.h"
7: 
8: static int passed = 0, failed = 0;
9: 
10: #define CHECK(cond, msg) do { \
11:     if (cond) { std::cout << "[PASS]" << msg << std::endl; passed++; } \
12:     else      { std::cout << "[FAIL]" << msg << std::endl; failed++; } \
13: } while (0)
14: 
15: // Test sizes: key 21, value 64
16: constexpr int KS = 21;
17: constexpr int VS = 64;
18: using BL = BlockList<KS, VS>;
19: 
20: // Pack a C string into a fixed VS buffer
21: static void packStr(const char* s, char* buf) {
22:     std::memset(buf, 0, VS);
23:     std::strncpy(buf, s, VS - 1);
24: }
25: 
26: // Insert helper: pack string value into fixed-size buffer
27: static bool insStr(BL& bl, const char* key, const char* val) {
28:     char buf[VS];
29:     packStr(val, buf);
30:     return bl.insert(key, buf);
31: }
32: 
33: // Integer key helper
34: static std::string k(int i) { return std::to_string(i); }
35: 
36: void testEmptyList() {
37:     std::remove("t_empty.bin");
38:     BL bl("t_empty.bin");
39:     char buf[VS];
40:     CHECK(!bl.find("1", buf), "EmptyList: find should return false");
41: }
42: 
43: void testInsertAndFind() {
44:     std::remove("t_data.bin");
45:     {
46:         BL bl("t_data.bin");
47:         CHECK(insStr(bl, "20", "twenty"), "insert 20");
48:         CHECK(insStr(bl, "10", "ten"),    "insert 10");
49:         CHECK(insStr(bl, "30", "thirty"), "insert 30");
50: 
51:         char buf[VS];
52:         CHECK(bl.find("20", buf) && std::strcmp(buf, "twenty") == 0, "find 20 -> twenty");
53:         CHECK(bl.find("10", buf) && std::strcmp(buf, "ten") == 0,    "find 10 -> ten");
54:         CHECK(bl.find("30", buf) && std::strcmp(buf, "thirty") == 0, "find 30 -> thirty");
55:         CHECK(!bl.find("25", buf), "find 25 -> not found");
56:     }
57:     {
58:         BL bl("t_data.bin");
59:         char buf[VS];
60:         CHECK(bl.find("20", buf) && std::strcmp(buf, "twenty") == 0, "reopen: find 20");
61:     }
62: }
63: 
64: void testInsertBasic() {
65:     std::remove("t_ins.bin");
66:     {
67:         BL bl("t_ins.bin");
68:         CHECK(insStr(bl, "20", "twenty"), "insert 20");
69:         CHECK(insStr(bl, "10", "ten"),    "insert 10");
70:         CHECK(insStr(bl, "30", "thirty"), "insert 30");
71:         CHECK(!insStr(bl, "20", "dup"),   "duplicate 20 -> false");
72: 
73:         char buf[VS];
74:         CHECK(bl.find("10", buf) && std::strcmp(buf, "ten") == 0,    "after insert, find 10");
75:         CHECK(bl.find("30", buf) && std::strcmp(buf, "thirty") == 0, "after insert, find 30");
76:     }
77:     {
78:         BL bl("t_ins.bin");
79:         char buf[VS];
80:         CHECK(bl.find("10", buf) && std::strcmp(buf, "ten") == 0,    "reopen: find 10");
81:         CHECK(bl.find("20", buf) && std::strcmp(buf, "twenty") == 0, "reopen: find 20");
82:         CHECK(bl.find("30", buf) && std::strcmp(buf, "thirty") == 0, "reopen: find 30");
83:     }
84: }
85: 
86: void testInsertSplit() {
87:     std::remove("t_split.bin");
88:     int n = CAPACITY + 5;
89:     {
90:         BL bl("t_split.bin");
91:         bool allIns = true;
92:         for (int i = 1; i <= n; i++) {
93:             char val[32];
94:             std::snprintf(val, sizeof(val), "val_%d", i);
95:             if (!insStr(bl, k(i).c_str(), val)) { allIns = false; break; }
96:         }
97:         CHECK(allIns, "split: insert CAPACITY+5 keys all succeed");
98:     }
99:     {
100:         BL bl("t_split.bin");
101:         char buf[VS];
102:         bool allFound = true;
103:         for (int i = 1; i <= n; i++) {
104:             if (!bl.find(k(i).c_str(), buf)) { allFound = false; break; }
105:         }
106:         CHECK(allFound, "split: reopen, all keys found");
107:         CHECK(bl.find("1", buf) && std::strcmp(buf, "val_1") == 0, "split: find(1) -> val_1");
108:         CHECK(bl.find(k(n).c_str(), buf), "split: find(last) ok");
109:     }
110: }
111: 
112: void testInsertReverse() {
113:     std::remove("t_rev.bin");
114:     int n = CAPACITY + 10;
115:     {
116:         BL bl("t_rev.bin");
117:         bool ok = true;
118:         for (int i = n; i >= 1; i--) {
119:             char val[32];
120:             std::snprintf(val, sizeof(val), "v%d", i);
121:             if (!insStr(bl, k(i).c_str(), val)) { ok = false; break; }
122:         }
123:         CHECK(ok, "reverse: all inserted");
124:     }
125:     {
126:         BL bl("t_rev.bin");
127:         char buf[VS];
128:         bool allFound = true;
129:         for (int i = 1; i <= n; i++) {
130:             if (!bl.find(k(i).c_str(), buf)) { allFound = false; break; }
131:         }
132:         CHECK(allFound, "reverse: reopen, all found");
133:     }
134: }
135: 
136: void testInsertRandom() {
137:     std::remove("t_rand.bin");
138:     int n = CAPACITY * 3 + 7;
139:     int keys[300];
140:     for (int i = 0; i < n; i++) keys[i] = i * 2;
141:     for (int i = n - 1; i > 0; i--) {
142:         int j = (i * 7 + 3) % (i + 1);
143:         int t = keys[i]; keys[i] = keys[j]; keys[j] = t;
144:     }
145:     {
146:         BL bl("t_rand.bin");
147:         bool ok = true;
148:         for (int i = 0; i < n; i++) {
149:             char val[32];
150:             std::snprintf(val, sizeof(val), "k%d", keys[i]);
151:             if (!insStr(bl, k(keys[i]).c_str(), val)) { ok = false; break; }
152:         }
153:         CHECK(ok, "random: all inserted");
154:     }
155:     {
156:         BL bl("t_rand.bin");
157:         char buf[VS];
158:         bool allFound = true;
159:         for (int i = 0; i < n; i++) {
160:             if (!bl.find(k(keys[i]).c_str(), buf)) { allFound = false; break; }
161:         }
162:         CHECK(allFound, "random: reopen, all found");
163:     }
164: }
165: 
166: void testErase() {
167:     std::remove("t_erase.bin");
168:     {
169:         BL bl("t_erase.bin");
170:         for (int i = 1; i <= 10; i++) {
171:             char v[16];
172:             std::snprintf(v, sizeof(v), "v%d", i);
173:             insStr(bl, k(i).c_str(), v);
174:         }
175:         CHECK(bl.erase("5"), "erase 5 -> true");
176:         CHECK(!bl.erase("5"), "erase 5 again -> false");
177:         CHECK(!bl.erase("999"), "erase 999 -> false");
178:         CHECK(!bl.erase("0"), "erase 0 (below min) -> false");
179: 
180:         char buf[VS];
181:         CHECK(!bl.find("5", buf), "5 gone after erase");
182:         CHECK(bl.find("4", buf) && std::strcmp(buf, "v4") == 0, "4 still there");
183:         CHECK(bl.find("6", buf) && std::strcmp(buf, "v6") == 0, "6 still there");
184:     }
185:     {
186:         BL bl("t_erase.bin");
187:         char buf[VS];
188:         CHECK(!bl.find("5", buf), "reopen: 5 still gone");
189:         CHECK(bl.find("6", buf),  "reopen: 6 still there");
190:     }
191: }
192: 
193: void testEraseUntilEmpty() {
194:     std::remove("t_erase2.bin");
195:     {
196:         BL bl("t_erase2.bin");
197:         for (int i = 1; i <= 10; i++) {
198:             char v[16];
199:             std::snprintf(v, sizeof(v), "v%d", i);
200:             insStr(bl, k(i).c_str(), v);
201:         }
202:         bool allErased = true;
203:         for (int i = 1; i <= 10; i++) {
204:             if (!bl.erase(k(i).c_str())) { allErased = false; break; }
205:         }
206:         CHECK(allErased, "erase all 10");
207:         CHECK(!bl.erase("1"), "erase after empty -> false");
208:     }
209: }
210: 
211: // Generic insert helper (different sizes)
212: template <int KS_, int VS_>
213: static bool insStrT(BlockList<KS_, VS_>& bl, const char* key, const char* val) {
214:     char buf[VS_];
215:     std::memset(buf, 0, VS_);
216:     std::strncpy(buf, val, VS_ - 1);
217:     return bl.insert(key, buf);
218: }
219: 
220: void testDifferentTypes() {
221:     std::remove("t_a.bin");
222:     std::remove("t_b.bin");
223:     BlockList<21, 64>  small("t_a.bin");
224:     BlockList<31, 128> large("t_b.bin");
225: 
226:     insStrT(small, "k", "v");
227:     insStrT(large, "a-much-longer-key", "a-much-longer-value-here");
228: 
229:     char buf[128];
230:     CHECK(small.find("k", buf) && std::strcmp(buf, "v") == 0, "small type: find k");
231:     CHECK(large.find("a-much-longer-key", buf)
232:           && std::strcmp(buf, "a-much-longer-value-here") == 0, "large type: find long key");
233: }
234: 
235: void testTraverse() {
236:     std::remove("t_trav.bin");
237:     BL bl("t_trav.bin");
238:     for (int i = 1; i <= 10; i++) {
239:         char v[16];
240:         std::snprintf(v, sizeof(v), "%d", i * 100);
241:         insStr(bl, k(i).c_str(), v);
242:     }
243: 
244:     int cnt = 0;
245:     std::string all;
246:     bl.traverse([&](const char* key, const char*) {
247:         all += key; all += ",";
248:         cnt++;
249:         return true;
250:     });
251:     CHECK(cnt == 10, "traverse: visited 10 entries");
252:     CHECK(all == "1,10,2,3,4,5,6,7,8,9,", "traverse: keys in lexicographic order");
253: 
254:     int n3 = 0;
255:     bl.traverse([&](const char*, const char*) {
256:         return ++n3 < 3;
257:     });
258:     CHECK(n3 == 3, "traverse: early stop after 3");
259: 
260:     std::string range;
261:     bl.traverseRange("3", "7", [&](const char* key, const char*) {
262:         range += key; range += ",";
263:         return true;
264:     });
265:     CHECK(range == "3,4,5,6,", "traverseRange[3,7): got 3,4,5,6");
266: 
267:     std::string pfx;
268:     bl.traverseRange("10", "11", [&](const char* key, const char*) {
269:         pfx += key; pfx += ",";
270:         return true;
271:     });
272:     CHECK(pfx == "10,", "traverseRange[10,11): only key '10'");
273: }
274: 
275: int main() {
276:     testEmptyList();
277:     testInsertAndFind();
278:     testInsertBasic();
279:     testInsertSplit();
280:     testInsertReverse();
281:     testInsertRandom();
282:     testErase();
283:     testEraseUntilEmpty();
284:     testDifferentTypes();
285:     testTraverse();
286: 
287:     std::cout << "\n==== Passed: " << passed
288:               << ", Failed: " << failed << "====\n";
289:     return failed == 0 ? 0 : 1;
290: }
291: 

(End of file - total 291 lines)
</content>