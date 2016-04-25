(ns editor.code-view-test
  (:require [clojure.test :refer :all]
            [dynamo.graph :as g]
            [editor.code-view :as cv :refer :all]
            [editor.code-view-ux :as cvx :refer :all]
            [editor.handler :as handler]
            [editor.lua :as lua]
            [editor.script :as script]
            [editor.gl.shader :as shader]
            [support.test-support :refer [with-clean-system tx-nodes]]))

(defn- setup-code-view-nodes [world source-viewer code code-node-type]
  (let [[code-node viewer-node] (tx-nodes (g/make-node world code-node-type)
                                          (g/make-node world CodeView :source-viewer source-viewer))]
    (do (g/transact (g/set-property code-node :code code))
        (setup-code-view viewer-node code-node 0)
        (g/node-value viewer-node :new-content)
        [code-node viewer-node])))

(deftest lua-syntax
  (with-clean-system
   (let [code ""
         opts lua/lua
         source-viewer (setup-source-viewer opts)
         [code-node viewer-node] (setup-code-view-nodes world source-viewer code script/ScriptNode)]
     (testing "default style"
       (let [new-code "x="]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= [{:start 0 :length 2 :stylename "default"}] (styles source-viewer)))))
     (testing "number style"
       (let [new-code "22"]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= [{:start 0 :length 2 :stylename "number"}] (styles source-viewer)))))
     (testing "keyword style"
       (let [new-code "break"]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= [{:start 0 :length 5 :stylename "keyword"}] (styles source-viewer)))))
     (testing "string style"
       (let [new-code "\"foo\""]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= [{:start 0 :length 5 :stylename "string"}] (styles source-viewer)))))
     (testing "single line comment style"
       (let [new-code "--foo"]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= [{:start 0 :length 5 :stylename "comment"}] (styles source-viewer)))))
     (testing "single line comment style stays to one line"
       (let [new-code "--foo\n bar"]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= {:start 0 :length 5 :stylename "comment"} (first (styles source-viewer))))))
     (testing "multi line comment style"
       (let [new-code "--[[\nmultilinecomment\n]]"]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= [{:start 0 :length 24 :stylename "comment-multi"}] (styles source-viewer)))))
     (testing "multi line comment style terminates"
       (let [new-code "--[[\nmultilinecomment\n]] morecode"]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= {:start 0 :length 24 :stylename "comment-multi"} (first (styles source-viewer)))))))))

(deftest glsl-syntax
  (with-clean-system
   (let [code ""
         opts (:code shader/glsl-opts)
         source-viewer (setup-source-viewer opts)
         [code-node viewer-node] (setup-code-view-nodes world source-viewer code shader/ShaderNode)]
     (testing "default style"
       (let [new-code "x="]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= [{:start 0 :length 2 :stylename "default"}] (styles source-viewer)))))
    (testing "number style"
       (let [new-code "22"]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= [{:start 0 :length 2 :stylename "number"}] (styles source-viewer)))))
    (testing "keyword style"
      (let [new-code "template"]
        (g/transact (g/set-property code-node :code new-code))
        (g/node-value viewer-node :new-content)
        (is (= [{:start 0 :length 8 :stylename "keyword"}] (styles source-viewer)))))
    (testing "string style"
      (let [new-code "\"foo\""]
        (g/transact (g/set-property code-node :code new-code))
        (g/node-value viewer-node :new-content)
        (is (= [{:start 0 :length 5 :stylename "string"}] (styles source-viewer)))))
     (testing "single line comment style"
       (let [new-code "//foo"]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= [{:start 0 :length 5 :stylename "comment"}] (styles source-viewer)))))
     (testing "single line comment style stays to one line"
       (let [new-code "//foo\n bar"]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= {:start 0 :length 5 :stylename "comment"} (first (styles source-viewer))))))
     (testing "multi line comment style"
       (let [new-code "/**multilinecomment\n**/"]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= [{:start 0 :length 23 :stylename "comment-multi"}] (styles source-viewer)))))
     (testing "multi line comment style terminates"
       (let [new-code "/**multilinecomment\n**/ more stuff"]
         (g/transact (g/set-property code-node :code new-code))
         (g/node-value viewer-node :new-content)
         (is (= {:start 0 :length 23 :stylename "comment-multi"} (first (styles source-viewer)))))))))

(defrecord TestClipboard [content]
  TextContainer
  (text! [this s] (reset! content s))
  (text [this] @content))

(defn- copy! [source-viewer clipboard]
  (handler/run :copy [{:name :code-view :env {:selection source-viewer :clipboard clipboard}}] {}))

(defn- paste! [source-viewer clipboard]
  (handler/run :paste [{:name :code-view :env {:selection source-viewer :clipboard clipboard}}]{}))

(deftest copy-paste-test
  (with-clean-system
    (let [clipboard (new TestClipboard (atom ""))
          code "hello world"
          opts lua/lua
          source-viewer (setup-source-viewer opts)
          [code-node viewer-node] (setup-code-view-nodes world source-viewer code script/ScriptNode)]
      (text-selection! source-viewer 6 5)
      (copy! source-viewer clipboard)
      (paste! source-viewer clipboard)
      (is (= "world" (text clipboard)))
      (is (= "worldhello world" (g/node-value code-node :code))))))

(defn- right! [source-viewer]
  (handler/run :right [{:name :code-view :env {:selection source-viewer}}]{}))

(defn- left! [source-viewer]
  (handler/run :left [{:name :code-view :env {:selection source-viewer}}]{}))

(deftest move-test
  (with-clean-system
    (let [code "hello world"
          opts lua/lua
          source-viewer (setup-source-viewer opts)
          [code-node viewer-node] (setup-code-view-nodes world source-viewer code script/ScriptNode)]
      (caret! source-viewer 5 false)
      (testing "moving right" (right! source-viewer)
               (g/node-value viewer-node :new-content)
               (is (= 6 (caret source-viewer)))
               (is (= 6 (g/node-value code-node :caret-position))))
      (testing "moving left"
        (left! source-viewer)
        (g/node-value viewer-node :new-content)
        (is (= 5 (caret source-viewer)))
        (is (= 5 (g/node-value code-node :caret-position))))
      (testing "out of bounds right"
        (caret! source-viewer (count code) false)
        (right! source-viewer)
        (right! source-viewer)
        (is (= (count code) (caret source-viewer))))
      (testing "out of bounds left"
        (caret! source-viewer 0 false)
        (left! source-viewer)
        (left! source-viewer)
        (is (= 0 (caret source-viewer)))))))

(defn- select-right! [source-viewer]
  (handler/run :select-right [{:name :code-view :env {:selection source-viewer}}]{}))

(defn- select-left! [source-viewer]
  (handler/run :select-left [{:name :code-view :env {:selection source-viewer}}]{}))

(deftest select-move-test
  (with-clean-system
    (let [code "hello world"
          opts lua/lua
          source-viewer (setup-source-viewer opts)
          [code-node viewer-node] (setup-code-view-nodes world source-viewer code script/ScriptNode)]
      (testing "selecting right"(select-right! source-viewer)
               (select-right! source-viewer)
               (g/node-value viewer-node :new-content)
               (is (= 2 (caret source-viewer)))
               (is (= 2 (g/node-value code-node :caret-position)))
               (is (= "he" (text-selection source-viewer))))
      (testing "selecting left"
        (select-left! source-viewer)
        (is (= 1 (caret source-viewer)))
        (is (= 1 (g/node-value code-node :caret-position)))
        (is (= "h" (text-selection source-viewer))))
      (testing "out of bounds right"
        (caret! source-viewer (count code) false)
        (select-right! source-viewer)
        (select-right! source-viewer)
        (is (= (count code) (caret source-viewer))))
      (testing "out of bounds left"
        (caret! source-viewer 0 false)
        (select-left! source-viewer)
        (select-left! source-viewer)
        (is (= 0 (caret source-viewer)))))))
